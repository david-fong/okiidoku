
#include "grid.h"

Game::Tile::Tile()           : fixedVal(false), biasIndex(0),      value(0) {}
Game::Tile::Tile(int rowLen) : fixedVal(false), biasIndex(rowLen), value(rowLen) {}

Game::Game(int order) : order(CLEAN_ORDER(order)), length(order * order), area(length * length) {
	grid = vector<Game::Tile>(area, Tile(length));

	rowBin = vector<occmask_t>(length, 0);
	colBin = vector<occmask_t>(length, 0);
	blkBin = vector<occmask_t>(length, 0);

	// Scramble each row's value-guessing-order:
	// *set the <length>'th entry to <length>
	for (int i = 0; i < length; i++) {
		vector<int> seq;
		for (int i = 0; i <= length; i++) seq.push_back(i);
		rowBiases.push_back(seq);
	}
	for (int i = 0; i < length; i++) {
		random_shuffle(rowBiases[i].begin(), prev(rowBiases[i].end()));
	}
}

void Game::init() {
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
		rowBin[row] &= valInv;
		colBin[col] &= valInv;
		blkBin[blk] &= valInv;
	}
	t->value = length;

	occmask_t invalidBin = rowBin[row] | colBin[col] | blkBin[blk];
	++(t->biasIndex) %= length + 1;
	for (; t->biasIndex < length; t->biasIndex++) {
		// If a valid value is found for this tile:
		t->value = rowBiases[row][t->biasIndex];
		const occmask_t valBit = 0b1 << t->value;
		if (!(invalidBin & valBit)) {
			rowBin[row] |= valBit;
			colBin[col] |= valBit;
			blkBin[blk] |= valBit;
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

void Game::generateSolution() {
	// Skip all seeded starting tiles:
	int i = 0;
	while (grid[i].fixedVal && i < area) i++;

	while (i < area) {
		if (isClear(setNextValid(i))) {
			while (--i >= 0 && grid[i].fixedVal);
		}
		else { while (++i < area && grid[i].fixedVal); }
	}
	// When done, set all values as fixed:
	for (int i = 0; i < area; i++) {
		grid[i].fixedVal = true;
	}
}

//void Game::print() {
//	const int W = 3;
//	char* str = calloc(area + length, W * sizeof(char));
//	for (int r = 0; r < length; r++) {
//		for (int c = 0; c < length; c++) {
//			sprintf(str + (r * (length + 1)) + c), "%*d", W, grid[r * length + c]);
//		}
//		sprintf(str + r + length), "%*.*s", W, W, '\n\n');
//	}
//	printf("%c\n", str);
//	free(str);
//	return;
//}
