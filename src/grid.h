#ifndef _GRID_H_
#define _GRID_H_

#include <locale>
using namespace std;


// Evaulates to <o> bounded to the range [3, 5]
#define CLEAN_ORDER(o) (((o) < 3) ? 3 : (((o) > 5) ? 5 : (o)))
#define GIVEUP_RATIO 20'000

/**
 * IMPORTANT:
 * - occmask_t  : mask width `order^2` bits.
 * - value_t    : uint range [0, order^2 +1].
 * - order_t    : uint range [0, order  ].
 * - length_t   : uint range [0, order^2].
 * - area_t     : uint range [0, order^4].
 */
typedef uint32_t occmask_t;
typedef  uint8_t value_t;
typedef  uint8_t order_t;
typedef  uint8_t length_t;
typedef uint16_t area_t;
typedef unsigned long long opcount_t;

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
        Tile(void);
        void clear(const value_t rowLen) {
            fixedVal = false;
            biasIndex = rowLen;
            value = rowLen;
        }
        bool fixedVal;
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
    Game(const order_t, ostream&, const bool isPretty);
    void runNew(void);
    void print(void) const;

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
    unsigned long totalGenCount;
    unsigned long successfulGenCount;
    ostream& outStream;
    const bool isPretty;

    void clear(void);
    // Returns the tile at index.
    Tile& setNextValid(const area_t index);
    // Generates a random solution. Returns the number of operations or
    // zero if the give-up threshold was reached. Any previous seeds must
    // not make generating a solution impossible.
    opcount_t generateSolution();

    void printMessageBar(const string&) const;

    // Seed all tiles of blocks along the main diagonal:
    area_t seed0(void);
    area_t seed0b(void);
    bool seed1Bitmask(const area_t index, const occmask_t min);
    // returns the number of additionally seeded tiles.
    area_t seed1(int ceiling);

    // Inline functions:
    bool isClear(const Tile& t) const { return t.biasIndex == length; }
    length_t getRow(const area_t index) const { return index / length; }
    length_t getCol(const area_t index) const { return index % length; }
    length_t getBlk(const area_t index) const { return getBlk(getRow(index), getCol(index)); }
    length_t getBlk(const length_t row, const length_t col) const { return (row / order) * order + (col / order); }

private:
    static const length_t seed1Constants[];
    static int myRandom (const int i) { return rand() % i; }
    struct MyNumpunct : numpunct<char> {
        string do_grouping() const {
            return "\03";
        }
    };
};

const length_t Game::seed1Constants[] = { 0, 0, 0, 0, 2, 9, };

#endif
