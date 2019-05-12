
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
typedef uint32_t occmask_t;

#define SEED0 seed0 // one of { seed0, seed0b, }

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
		bool  fixedVal;
		short biasIndex;
		short value;
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
	const int order;
	const int length;
	const int area;
	vector<Tile> grid;
	vector<occmask_t> rowBins;
	vector<occmask_t> colBins;
	vector<occmask_t> blkBins;
	vector<vector<short>> rowBiases;

	// Solution generation methods: --------------------------------------------------
	void clear();
	// Returns the tile at index.
	Tile* setNextValid(const int index);
	int   seed1Bitmask(const int index, const occmask_t min);
	// Seed all tiles of blocks along the main diagonal:
	void seed0();
	void seed0b();
	int  seed1(int ceiling);
	// Generates a random solution.
	// Any previous seeds must not make generating a solution impossible.
	void generateSolution();

	// Inline functions: -------------------------------------------------------------
	bool isClear(Tile const * t) const { return t->biasIndex == length; }
	int getRow(int index) const { return index / length; }
	int getCol(int index) const { return index % length; }
	int getBlk(int index) const { return getBlk(getRow(index), getCol(index)); }
	int getBlk(int row, int col) const { return (row / order) * order + (col / order); }
	
//////////////////////////////////////////////////////////////////////////////////////
}; // ================================================================================
#endif
