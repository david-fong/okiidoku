
/**
 * 
 */
#ifndef _GRID_H_
#define _GRID_H_

#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;

#define MIN_ORDER 3
#define MAX_ORDER 5
typedef int32_t occmask_t;

/**
 * order limited to range [MIN_ORDER, MAX_ORDER].
 */
class Game { // ======================================================================
//////////////////////////////////////////////////////////////////////////////////////
private:
	/**
	 * When clear, biasIndex is the parent Game's length (same for value).
	 */
	class Tile {
	public:
		Tile(int rowLength);
		bool canChange;
		int  biasIndex;
		int  value;
	};


public: // ===========================================================================
	/**
	 * 
	 */
	Game(int order);

	&ostream operator<<(ostream& os, const Game& game);


private: // ==========================================================================
	// Private fields: ---------------------------------------------------------------
	int order, length, area;
	Tile *const grid;
	vector<occmask_t> rowBin, colBin, blkBin;
	vector<vector<int>> rowBiases;

	// Solution generation methods: --------------------------------------------------
	// Returns the tile at index.
	Tile* setNextValid(int index);
	// Seed all tiles of blocks along the main diagonal:
	void seed0();

	// Inline functions: -------------------------------------------------------------
	bool isClear(Tile const * t) { return t->biasIndex == length; }
	int getRow(int index) { return index / length; }
	int getCol(int index) { return index % length; }
	int getBlk(int index) { return getBlk(getRow(index), getCol(index)); }
	int getBlk(int row, int col) { return (row / order) * order + (col / order); }
	
//////////////////////////////////////////////////////////////////////////////////////
}; // ================================================================================
#endif
