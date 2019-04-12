
#include "grid.h"

Game::Tile::Tile()           : fixedVal(false), biasIndex(0),      value(0) {}
Game::Tile::Tile(int rowLen) : fixedVal(false), biasIndex(rowLen), value(rowLen) {}

Game::Game(int order) : order(CLEAN_ORDER(order)), length(order * order), area(length * length) {
	grid    = vector<Tile>(area);
	rowBins = vector<occmask_t>(length);
	colBins = vector<occmask_t>(length);
	blkBins = vector<occmask_t>(length);

	for (int i = 0; i < length; i++) {
		vector<int> seq;
		for (int i = 0; i <= length; i++) seq.push_back(i);
		rowBiases.push_back(seq);
	}
}

void Game::runNew() {
	// Initialize all values as empty:
	fill(grid.begin(), grid.end(), Tile(length));
	fill(rowBins.begin(), rowBins.end(), 0);
	fill(colBins.begin(), colBins.end(), 0);
	fill(blkBins.begin(), blkBins.end(), 0);

	// Scramble each row's value-guessing-order:
	// *set the <length>'th entry to <length>
	for (int i = 0; i < length; i++) {
		random_shuffle(rowBiases[i].begin(), rowBiases[i].end() - 1);
	}
	// Generate a solution:
	seed0();
	generateSolution();
}


// TODO: change col and blk to pointers to their bins:
Game::Tile* Game::setNextValid(int index) {
	const int row = getRow(index);
	const int col = getCol(index);
	const int blk = getBlk(row, col);

	// If the tile is currently already set, clear it:
	Tile* t = &grid[index];
	if (t->biasIndex != length) {
		const int valInv = ~(0b1 << t->value);
		rowBins[row] &= valInv;
		colBins[col] &= valInv;
		blkBins[blk] &= valInv;
	}
	t->value = length;

	occmask_t invalidBin = rowBins[row] | colBins[col] | blkBins[blk];
	++(t->biasIndex) %= length + 1;
	for (; t->biasIndex < length; t->biasIndex++) {
		// If a valid value is found for this tile:
		t->value = rowBiases[row][t->biasIndex];
		const occmask_t valBit = 0b1 << t->value;
		if (!(invalidBin & valBit)) {
			rowBins[row] |= valBit;
			colBins[col] |= valBit;
			blkBins[blk] |= valBit;
			break;
		}
	}
	return t;
}

void Game::seed0() {
	const int bRow = order * length;
	for (int b = 0; b < area; b += bRow + order)
		for (int r = 0; r < bRow; r += length)
			for (int c = 0; c < order; c++)
				setNextValid(b + r + c)->fixedVal = true;
}

int Game::seed1() {
	int count = 0;
	vector<short> possibilities;
	return count;
}

void Game::generateSolution() {
	// Skip all seeded starting tiles:
	int i = 0;
	while (grid[i].fixedVal && i < area) i++;

	while (i < area) {
		// Push a new permutation:
		if (isClear(setNextValid(i))) {

			// Pop and step backward:
			do {
				// Fail if no solution could be found:
				if (--i < 0) throw Game::OPseed;
			} while (grid[i].fixedVal);
		}
		// Step forward to push a new permutation:
		else { while (++i < area && grid[i].fixedVal); }
	}
	// When done, set all values as fixed:
	for (int i = 0; i < area; i++) {
		grid[i].fixedVal = true;
	}
}

void Game::print() {
	for (int i = 0; i < area; i++) {
		Tile* t = &grid[i];
		const string pad((i % length == 0) ? "\n " : " ");
		cout << pad << (isClear(t) ? -1: t->value);
	}
	cout << endl;
}
