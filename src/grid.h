#ifndef _GRID_H_
#define _GRID_H_

using namespace std;


// Evaulates to <o> bounded to the range [3, 5]
#define CLEAN_ORDER(o) (((o) < 3) ? 3 : (((o) > 5) ? 5 : (o)))

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
    Game(const order_t, ostream&);
    void runNew(void);
    void print(void) const;
    void prettyPrint(void) const;

private:
    // Private fields:
    ostream& outStream;
    const int order;
    const int length;
    const int area;
    vector<Tile> grid;
    vector<occmask_t> rowBins;
    vector<occmask_t> colBins;
    vector<occmask_t> blkBins;
    vector<vector<value_t>> rowBiases;

    // Solution generation methods:
    void clear(void);
    // Returns the tile at index.
    Tile& setNextValid(const area_t index);
    // Seed all tiles of blocks along the main diagonal:
    area_t seed0(void);
    area_t seed0b(void);
    bool seed1Bitmask(const area_t index, const occmask_t min);
    // returns the number of additionally seeded tiles.
    area_t seed1(int ceiling);
    // Generates a random solution.
    // Any previous seeds must not make generating a solution impossible.
    opcount_t generateSolution();

    // Inline functions:
    bool isClear(const Tile& t) const { return t.biasIndex == length; }
    length_t getRow(const area_t index) const { return index / length; }
    length_t getCol(const area_t index) const { return index % length; }
    length_t getBlk(const area_t index) const { return getBlk(getRow(index), getCol(index)); }
    length_t getBlk(const length_t row, const length_t col) const { return (row / order) * order + (col / order); }

private:
    static const length_t seed1Constants[];
    static int myRandom (const int i) { return rand() % i; }
};

const length_t Game::seed1Constants[] = { 0, 0, 0, 0, 2, 9, };

#endif
