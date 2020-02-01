#ifndef _GRID_H_
#define _GRID_H_

#include <locale>
#include <vector>
#include <map>


// Evaluates to <o> bounded to the range [2, 5]
#define CLEAN_ORDER(o) (((o) < 2) ? 2 : (((o) > 5) ? 5 : (o)))
#define GIVEUP_RATIO 128
// TODO ^consumer code assumes num operations is proportional to area^2. it that true?

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


/**
 * 
 */
class Game {
public:
    /**
     * When clear, biasIndex is the parent Game's length (same for value).
     */
    class Tile {
        friend Game;
    public:
        explicit Tile(const area_t index);
        void clear(const value_t rowLen) {
            biasIndex = rowLen;
            value = rowLen;
            fixedVal = false;
        }
        const area_t index;
        friend void swap(Tile& a, Tile& b)
        {
            using std::swap;
            swap(a.biasIndex, b.biasIndex);
            swap(a.value, b.value);
            swap(a.fixedVal, b.fixedVal);
        }
    protected:
        value_t biasIndex;
        value_t value;
        bool fixedVal;
    };

public:
    Game(const order_t, std::ostream&, const bool isPretty);

    // return false if command is to exit the program:
    bool runCommand(const std::string& cmdLine);
    void runNew(void);
    void runMultiple(unsigned int);
    void print(void) const;

private:
    // Private fields:
    const int order;
    const int length;
    const int area;
    std::vector<Tile> grid;
    std::vector<occmask_t> rowBins;
    std::vector<occmask_t> colBins;
    std::vector<occmask_t> blkBins;
    std::vector<std::vector<value_t>> rowBiases;
    std::vector<Tile> traversalPath;
    unsigned long totalGenCount;
    unsigned long successfulGenCount;
    std::ostream& outStream;
    const bool isPretty;

    void clear(void);
    void seed(const bool printInfo);
    // Returns the tile at index.
    Tile& setNextValid(const area_t index);
    // Generates a random solution. Returns the number of operations or
    // zero if the give-up threshold was reached. Any previous seeds must
    // not make generating a solution impossible.
    opcount_t generateSolution();

    length_t tileNumCandidates(const area_t) const;
    void printMessageBar(std::string const&) const;

    // Seed all tiles of blocks along the main diagonal:
    area_t seed0(void);
    area_t seed0b(void);
    bool seed1Bitmask(const area_t index, const occmask_t min);
    // returns the number of additionally seeded tiles.
    area_t seed1(int ceiling);

    // Inline functions:
    bool isClear(Tile const& t) const { return t.biasIndex == length; }
    length_t getRow(const area_t index) const { return index / length; }
    length_t getCol(const area_t index) const { return index % length; }
    length_t getBlk(const area_t index) const { return getBlk(getRow(index), getCol(index)); }
    length_t getBlk(const length_t row, const length_t col) const { return (row / order) * order + (col / order); }

public:
    class OPseedException : public std::exception {
        virtual const char* what() const throw() {
            return "seed methods made it impossible to generate any solutions.";
        }
    } OPseed;

    typedef enum { HELP, QUIT, RUN_SINGLE, RUN_MULTIPLE, } Command;
    static const std::map<std::string, Command> COMMAND_MAP;
    static const std::string HELP_MESSAGE;
    static const std::string REPL_PROMPT;

private:
    static const length_t seed1Constants[];
    static int myRandom (const int i) { return rand() % i; }
    struct MyNumpunct : std::numpunct<char> {
        std::string do_grouping() const {
            return "\03";
        }
    };
};



const std::map<std::string, Game::Command> Game::COMMAND_MAP = {
    { "help", HELP },
    { "quit", QUIT },
    { "", RUN_SINGLE },
    { "trials", RUN_MULTIPLE }
};
const std::string Game::HELP_MESSAGE = "\nCOMMAND MENU:"
    "\n- help"
    "\n- quit"
    "\n- <enter>"
    "\n- trials <n>"
    "\n";
const std::string Game::REPL_PROMPT = "\n> ";
const length_t Game::seed1Constants[] = { 0, 0, 0, 0, 2, 9, };

#endif