
#include "grid.h"

Game::Tile::Tile(int rowLength)
	:canChange(true),biasIndex(rowLength),value(rowLength) {}

Game::Game(int order) {
	if (order < MIN_ORDER) order = MIN_ORDER;
	if (order > MAX_ORDER) order = MAX_ORDER;
	length = order  * order;
	area   = length * length;
	grid   = calloc(area, sizeof(Tile));
	for (int i = 0; i < area; i++) {
		grid[i] = Tile(length);
	}

	rowBin = vector<occmask_t>(length, 0);
	colBin = vector<occmask_t>(length, 0);
	blkBin = vector<occmask_t>(length, 0);

	// Scramble each row's value-guessing-order:
	// *set the <length>'th entry to <length>
	vector<const int> seq;
	for (int i = 0; i <= length) seq.push_back(i);
	rowBiases = vector<vector<int>>(length, seq);
	for (int i = 0; i < length; i++) {
		random_shuffle(rowBiases[i].begin(), prev(rowBiases[i].end()));
	}
}

Game::Tile* Game::setNextValid(int index) {
	const row = getRow(index);
	const col = getCol(index);
	const blk = getBlk(row, col);

	// If the tile is currently already set, clear it:
	Tile *const t = grid + index;
	if (t->biasIndex != length) {
		const ~valInv = 0b1 << t->value;
		rowBin[row] &= valInv;
		colBin[col] &= valInv;
		blkBin[blk] &= valInv;
	}
	t->value = length;

	occmask_t invalidBin = rowBin[row] | colBin[col] | blkBin[blk];
	++(t->biasIndex) %= length;
	for (; t->biasIndex < length; t->biasIndex++) {
		// If a valid value is found for this tile:
		t->value = rowBiases[t->biasIndex];
		const valBit = 0b1 << t->value;
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
	const bRow = order * length;
	for (int b = 0; b < area; b += bRow + length) {
		for (int r = 0; r < bRow; r++) {
			for (int c = 0; c < order; c++) {
				setRandomValid(b + r + c)->canChange = false;
			}
		}
	}
}
