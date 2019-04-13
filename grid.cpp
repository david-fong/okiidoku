
#include "grid.h"

Game::Tile::Tile()           : fixedVal(false), biasIndex(0),      value(0) {}
Game::Tile::Tile(int rowLen) : fixedVal(false), biasIndex(rowLen), value(rowLen) {}

Game::Game(int order) : order(CLEAN_ORDER(order)), length(order * order), area(length * length) {
	grid    = vector<Tile>(area);
	rowBins = vector<occmask_t>(length);
	colBins = vector<occmask_t>(length);
	blkBins = vector<occmask_t>(length);

	for (int i = 0; i < length; i++) {
		vector<short> seq;
		for (short i = 0; i <= length; i++) seq.push_back(i);
		rowBiases.push_back(seq);
	}
}

void Game::clear() {
	// Initialize all values as empty:
	fill(grid.begin(), grid.end(), Tile(length));
	fill(rowBins.begin(), rowBins.end(), 0);
	fill(colBins.begin(), colBins.end(), 0);
	fill(blkBins.begin(), blkBins.end(), 0);
}

void Game::runNew() {
	clear();

	// Scramble each row's value-guessing-order:
	// *set the <length>'th entry to <length>
	for (int i = 0; i < length; i++) {
		random_shuffle(rowBiases[i].begin(), rowBiases[i].end() - 1);
	}
	// Generate a solution:
	SEED0();
	seed1(2 * order + 1);
	generateSolution();
}


Game::Tile* Game::setNextValid(const int index) {
	const int row = getRow(index);
	occmask_t *const colBin = &colBins[getCol(index)];
	occmask_t *const blkBin = &blkBins[getBlk(index)];

	// If the tile is currently already set, clear it:
	Tile* t = &grid[index];
	if (t->biasIndex != length) {
		const occmask_t valInv = ~(0b1 << t->value);
		rowBins[row] &= valInv;
		*colBin &= valInv;
		*blkBin &= valInv;
	}
	t->value = length;

	occmask_t invalidBin = rowBins[row] | *colBin | *blkBin;
	++(t->biasIndex) %= length + 1;
	for (; t->biasIndex < length; t->biasIndex++) {
		// If a valid value is found for this tile:
		t->value = rowBiases[row][t->biasIndex];
		const occmask_t valBit = 0b1 << t->value;
		if (!(invalidBin & valBit)) {
			rowBins[row] |= valBit;
			*colBin |= valBit;
			*blkBin |= valBit;
			break;
		}
	}
	return t;
}

int Game::seed1Bitmask(const int index, const occmask_t min) {
	occmask_t* row = &rowBins[getRow(index)];
	occmask_t* col = &colBins[getCol(index)];
	occmask_t* blk = &blkBins[getBlk(index)];

	if ((*row | *col | *blk) <= min) {
		// seed this tile:
		grid[index].fixedVal = true;
		*row = (*row << 1) | 0b1;
		*col = (*col << 1) | 0b1;
		*blk = (*blk << 1) | 0b1;
		return 1;
	}
	return 0;
}

void Game::seed0() {
	const int bRow = order * length;
	for (int b = 0; b < area; b += bRow + order)
		for (int r = 0; r < bRow; r += length)
			for (int c = 0; c < order; c++)
				setNextValid(b + r + c)->fixedVal = true;
}

void Game::seed0b() {
	// Get offsets to blocks along top and left edge:
	vector<int> blocks;
	int bRow = order * length;
	for (int i = 0; i < length; i += order)
		blocks.push_back(i);
	for (int i = bRow; i < area; i += bRow)
		blocks.push_back(i);

	for (int i = 0; i < blocks.size(); i++)
		for (int r = 0; r < bRow; r += length)
			for (int c = 0; c < order; c++)
				setNextValid(blocks[i] + r + c)->fixedVal = true;
}

int Game::seed1(int ceiling) {
	ceiling = -1 << (ceiling - 1);
	Game occ(order);
	occ.clear();

	// Scan forward past those already seeded.
	for (int i = 0; i < area; i++) {
		if (grid[i].fixedVal) { occ.seed1Bitmask(i, -1); }
	}
	int i = -1;
	while (grid[++i].fixedVal);
	occmask_t min = (
		occ.rowBins[getRow(i)] |
		occ.colBins[getCol(i)] |
		occ.blkBins[getBlk(i)] );

	while (!(min & ceiling)) {
		occ.seed1Bitmask(i, min);

		// Advance and wrap-around if necessary:
		while (++i < area && occ.grid[i].fixedVal);
		if (i == area) { i = 0; min <<= 1; min |= 0b1; }
	}

	int count = 0;
	for (int i = 0; i < area; i++) {
		if (occ.grid[i].fixedVal && !grid[i].fixedVal) {
			count++;
			setNextValid(i)->fixedVal = true;;
		}
	}
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
				if (--i < 0) { throw Game::OPseed; }
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
