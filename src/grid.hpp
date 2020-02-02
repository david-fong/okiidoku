#ifndef _GRID_H_
#define _GRID_H_

#include <vector>
#include <map>


// Evaluates to <o> bounded to the range [2, 5]
#define CLEAN_ORDER(o) (((o) < 2) ? 2 : (((o) > 5) ? 5 : (o)))
#define GIVEUP_RATIO 0.5
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
class Sudoku {
public:
    /**
     * When clear, biasIndex is the parent Sudoku's length (same for value).
     */
    class Tile {
        friend Sudoku;
    public:
        Tile(void) = delete;
        explicit Tile(const area_t index):
            index       (index),
            biasIndex   (0),
            value       (0) {}
        void clear(const value_t rowLen) {
            biasIndex = rowLen;
            value = rowLen;
        }
        bool operator<(Tile const& other) const {
            return index < other.index;
        }
    protected:
        area_t index;
        value_t biasIndex;
        value_t value; // undefined if clear.
    };


public:
    const int order;
    const int length;
    const int area;
    Sudoku(const order_t, std::ostream&, const bool isPretty);

    // return false if command is to exit the program:
    bool runCommand(const std::string& cmdLine);
    void runNew(void);
    void runMultiple(const unsigned int);
    void print(void) const;
    void printMessageBar(std::string const&, unsigned int, const char = '=') const;
    void printMessageBar(std::string const&, const char = '=') const;

private:
    // Private fields:
    std::vector<Tile> grid;
    std::vector<bool> seeds;
    std::vector<occmask_t> rowBins;
    std::vector<occmask_t> colBins;
    std::vector<occmask_t> blkBins;
    std::vector<std::vector<value_t>> rowBiases;
    unsigned long totalGenCount;
    unsigned long successfulGenCount;

    bool doSeeding;
    std::ostream& outStream;
    const bool isPretty;
    const unsigned int statsWidth;

    void clear(void);
    void seed(const bool printInfo);
    // Generates a random solution. Returns the number of operations or
    // zero if the give-up threshold was reached. Any previous seeds must
    // not make generating a solution impossible.
    opcount_t generateSolution();
    // Returns the tile at index.
    Tile& setNextValid(const area_t index);
    length_t tileNumNonCandidates(const area_t) const;

    area_t seed0(void);
    bool seed1Bitmask(const area_t index, const occmask_t min);
    // returns the number of additionally seeded tiles.
    area_t seed1(int ceiling);

    // Inline functions:
    bool isClear(Tile const& t) const { return t.biasIndex == length; }
    length_t getRow(const area_t index) const { return index / length; }
    length_t getCol(const area_t index) const { return index % length; }
    length_t getBlk(const area_t index) const { return getBlk(getRow(index), getCol(index)); }
    length_t getBlk(const length_t row, const length_t col) const { return ((row / order) * order) + (col / order); }

public:
    class OPseedException : public std::exception {
        virtual const char* what() const throw() {
            return "seed methods made it impossible to generate any solutions.";
        }
    } OPseed;

    typedef enum { HELP, QUIT, RUN_SINGLE, RUN_MULTIPLE, TOGGLE_SEEDING, } Command;
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



const std::map<std::string, Sudoku::Command> Sudoku::COMMAND_MAP = {
    { "help", HELP },
    { "quit", QUIT },
    { "", RUN_SINGLE },
    { "trials", RUN_MULTIPLE },
    { "seed", TOGGLE_SEEDING }
};
const std::string Sudoku::HELP_MESSAGE = "\nCOMMAND MENU:"
    "\n- help           print this help menu."
    "\n- quit           terminate this program."
    "\n- {enter}        generate a single solution."
    "\n- trials <n>     generate <n> solutions."
    "\n- seed           toggle whether seeding is performed."
    ;
const std::string Sudoku::REPL_PROMPT = "\n> ";
const length_t Sudoku::seed1Constants[] = { 0, 0, 0, 0, 4+2, 5+4, };

#endif