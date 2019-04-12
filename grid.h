
/**
 * 
 */
#ifndef _GRID_H_
#define _GRID_H_

#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

// Evaulates to <o> bounded to the range [3, 5]
#define CLEAN_ORDER(o) ((o < 3) ? 3 : ((o > 5) ? 5 : o))

// IMPORTANT: can only handle length <= 32 (ie. order <= floor(sqrt(32)))
typedef int32_t occmask_t;

/**
 * 
 */
class Game { // ======================================================================
//////////////////////////////////////////////////////////////////////////////////////
private:
	/**
	 * When clear, biasIndex is the parent Game's length (same for value).
	 */
	class Tile {
	public:
		Tile();
		Tile(int rowLen);
		bool fixedVal;
		int  biasIndex;
		int  value;
	};


public: // ===========================================================================
	/**
	 * 
	 */
	class OPseedException: public exception {
		virtual const char* what() const throw() {
			return "seed methods made it impossible to generate any solutions.";
		}
	} OPseed;

	// Constructor:
	Game(int order);
	void runNew();
	void print();


private: // ==========================================================================
	// Private fields: ---------------------------------------------------------------
	const int order, length, area;
	vector<Tile> grid;
	vector<occmask_t> rowBins, colBins, blkBins;
	vector<vector<int>> rowBiases;

	// Solution generation methods: --------------------------------------------------
	// Returns the tile at index.
	Tile* setNextValid(int index);
	// Seed all tiles of blocks along the main diagonal:
	void seed0();
	int  seed1();
	// Generates a random solution.
	// Any previous seeds must not make generating a solution impossible.
	void generateSolution();

	// Inline functions: -------------------------------------------------------------
	bool isClear(Tile const * t) { return t->biasIndex == length; }
	int getRow(int index) { return index / length; }
	int getCol(int index) { return index % length; }
	int getBlk(int index) { return getBlk(getRow(index), getCol(index)); }
	int getBlk(int row, int col) { return (row / order) * order + (col / order); }
	
//////////////////////////////////////////////////////////////////////////////////////
}; // ================================================================================
#endif
