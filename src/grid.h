#ifndef _GRID_H_
#define _GRID_H_

#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

// Evaulates to <o> bounded to the range [3, 5]
#define CLEAN_ORDER(o) (((o) < 3) ? 3 : (((o) > 5) ? 5 : (o)))

/**
 * IMPORTANT:
 * - occmask_t  : mask width `order^2` bits.
 * - value_t    : uint range [0, order^2].
 * - length_t   : uint range [0, order^2].
 * - area_t     : uint range [0, order^4].
 */
typedef uint32_t occmask_t;
typedef  uint8_t value_t;
typedef  uint8_t length_t;
typedef uint16_t area_t;

#define SEED0 seed0 // one of { seed0, seed0b, }


/**
 * 
 */
class Game {
private:
    /**
     * When clear, biasIndex is the parent Game's length (same for value).
     */
    class Tile {
    public:
        Tile();
        Tile(const value_t rowLen);
        bool    fixedVal;
        value_t biasIndex;
        value_t value;
    };

public:
    /**
     * 
     */
    class OPseedException: public exception {
        virtual const char* what() const throw() {
            return "seed methods made it impossible to generate any solutions.";
        }
    } OPseed;

    // Constructor:
    Game(const length_t order);
    void runNew();
    void print();

private:
    // Private fields:
    const int order;
    const int length;
    const int area;
    vector<Tile> grid;
    vector<occmask_t> rowBins;
    vector<occmask_t> colBins;
    vector<occmask_t> blkBins;
    vector<vector<value_t>> rowBiases;

    // Solution generation methods:
    void clear();
    // Returns the tile at index.
    Tile* setNextValid(const area_t index);
    int   seed1Bitmask(const area_t index, const occmask_t min);
    // Seed all tiles of blocks along the main diagonal:
    void seed0();
    void seed0b();
    int  seed1(int ceiling);
    // Generates a random solution.
    // Any previous seeds must not make generating a solution impossible.
    void generateSolution();

    // Inline functions: -------------------------------------------------------------
    bool isClear(Tile const *const t) const { return t->biasIndex == length; }
    length_t getRow(const area_t index) const { return index / length; }
    length_t getCol(const area_t index) const { return index % length; }
    length_t getBlk(const area_t index) const { return getBlk(getRow(index), getCol(index)); }
    length_t getBlk(const length_t row, const length_t col) const { return (row / order) * order + (col / order); }
};

#endif
