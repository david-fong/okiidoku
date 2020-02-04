#ifndef _GRID_H_
#define _GRID_H_

#include <vector>
#include <map>


#define GIVEUP_THRESH_COEFF 0.7
// TODO ^consumer code assumes num operations is proportional to area^2. it that true?


/**
 * Note: Since most of my static members don't depend on the template
 * parameter, I moved them out to this namespace.
 */
namespace Sudoku {

    typedef enum { ROW_MAJOR, BLOCK_COLS, GenPath_MAX = BLOCK_COLS, } GenPath;
    const std::vector<std::string> GenPath_Names = {
        "rowmajor",
        "blockcol",
    };
    typedef enum { HELP, QUIT, RUN_SINGLE, RUN_MULTIPLE, SET_GENPATH, } Command;
    const std::map<std::string, Command> COMMAND_MAP = {
        { "help",       HELP },
        { "quit",       QUIT },
        { "",           RUN_SINGLE },
        { "trials",     RUN_MULTIPLE },
        { "genpath",    SET_GENPATH },
    };
    const std::string HELP_MESSAGE = "\nCOMMAND MENU:"
        "\n- help           print this help menu."
        "\n- quit           terminate this program."
        "\n- {enter}        generate a single solution."
        "\n- trials <n>     generate <n> solutions."
        "\n- genpath        cycle generator traversal path."
        ;
    const std::string REPL_PROMPT = "\n> ";

    typedef enum { TWO = 2, THREE = 3, FOUR = 4, FIVE = 5, } Order;
    const std::vector<Order> OrderVec = { TWO, THREE, FOUR, FIVE, };

    /**
     * 
     */
    template <Order O>
    class Solver {

    public:
        // TODO: use std::conditional.
        // TODO: later: try using std::bitset from <numeric>
        typedef uint32_t occmask_t; // mask width `order^2` bits.
        typedef  uint8_t value_t;   // uint range [0, order^2].
        typedef  uint8_t order_t;   // uint range [0, order  ].
        typedef  uint8_t length_t;  // uint range [0, order^2].
        typedef uint16_t area_t;    // uint range [0, order^4].
        typedef unsigned long long opcount_t;

    public:
        /**
         * When clear, biasIndex is the parent Solver's length and value
         * is undefined.
         */
        class Tile {
            friend Solver;
        public:
            void clear(const value_t rowLen) noexcept {
                biasIndex = rowLen;
                value = rowLen;
            }
        protected:
            value_t biasIndex;
            value_t value; // undefined if clear.
        };


    public:
        const order_t   order;
        const length_t  length;
        const area_t    area;
        explicit Solver(std::ostream&);

        // Return false if command is to exit the program:
        bool runCommand(const std::string& cmdLine);
        void runNew(void);
        void runMultiple(const unsigned int);
        void print(void) const;
        void printMessageBar(std::string const&, unsigned int, const char = '=') const;
        void printMessageBar(std::string const&, const char = '=') const;

    private:
        std::vector<Tile> grid;
        std::vector<occmask_t> rowSymbolOccMasks;
        std::vector<occmask_t> colSymbolOccMasks;
        std::vector<occmask_t> blkSymbolOccMasks;
        std::vector<std::vector<value_t>> rowBiases;
        std::vector<area_t> traversalOrder;
        GenPath genPath;
        unsigned long totalGenCount;
        unsigned long successfulGenCount;

        std::ostream& os;
        const bool isPretty;
        const unsigned int statsWidth;

        void clear(void);
        void seed(const bool printInfo);
        // Generates a random solution. Returns the number of operations or
        // zero if the give-up threshold was reached. Any previous seeds must
        // not make generating a solution impossible.
        opcount_t generateSolution();
        // Returns the tile at index.
        Tile const& setNextValid(const area_t index);
        length_t tileNumNonCandidates(const area_t) const noexcept;
        void setGenPath(const GenPath) noexcept;

        // Inline functions:
        bool isClear(Tile const& t) const noexcept { return t.biasIndex == length; }
        length_t getRow(const area_t index) const noexcept { return index / length; }
        length_t getCol(const area_t index) const noexcept { return index % length; }
        length_t getBlk(const area_t index) const noexcept { return getBlk(getRow(index), getCol(index)); }
        length_t getBlk(const length_t row, const length_t col) const noexcept { return ((row / order) * order) + (col / order); }

    private:
        static int myRandom (const int i) { return rand() % i; }
        struct MyNumpunct : std::numpunct<char> {
            std::string do_grouping() const {
                return "\03";
            }
        };
    };

} // End of Sudoku namespace

#endif