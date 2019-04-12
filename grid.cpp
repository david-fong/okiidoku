
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
	SEED0();
	generateSolution();
}


Game::Tile* Game::setNextValid(const int index) {
	const int row = getRow(index);
	occmask_t *const colBin = &colBins[getCol(index)];
	occmask_t *const blkBin = &blkBins[getBlk(index)];

	// If the tile is currently already set, clear it:
	Tile* t = &grid[index];
	if (t->biasIndex != length) {
		const int valInv = ~(0b1 << t->value);
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

int Game::seed1() {
	int count = 0;

	class Capacity {
	public:
		Capacity(Game& ctx) : ctx(ctx) {
			const occmask_t initialCpct = (0b1 << (ctx.length + 1)) - 1;
			rowCpct = vector<occmask_t>(ctx.length, initialCpct);
			colCpct = vector<occmask_t>(ctx.length, initialCpct);
			blkCpct = vector<occmask_t>(ctx.length, initialCpct);

			buckets = vector<vector<int>>(ctx.length, vector<int>());
			for (int i = 0; i < ctx.area; i++) {
				buckets[ctx.length - 1].push_back(i);
			}
			// Take cencus of previous seeds:
			for (int i = 0; i < ctx.area; i++) {
				if (ctx.grid[i].fixedVal) set(i);
			}

			//for (int i = 0; i < ctx.area; i++) {
			//	// Take the minimum of the worst-case
			//	// context capacities in bit-bar format:
			//	const occmask_t cpct = (
			//		rowCpct[ctx.getRow(i)] &
			//		colCpct[ctx.getCol(i)] &
			//		blkCpct[ctx.getBlk(i)]);
			//}
		}
		void set(int index) {
			rowCpct[ctx.getRow(index)] >>= 1;
			colCpct[ctx.getCol(index)] >>= 1;
			blkCpct[ctx.getBlk(index)] >>= 1;
			// For each affected tile, update its bucket in the buckets field:
			// TODO: since I'll need to go through each affected tile, I might as
			//  well go back to implementing this as a 2D array of int's :|
		}
	private:
		const Game& ctx;
		vector<vector<int>> buckets;
		vector<occmask_t> rowCpct, colCpct, blkCpct;
	} capacity(*this);

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
