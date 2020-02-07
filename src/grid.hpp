#ifndef _GRID_H_
#define _GRID_H_

#include <array>
#include <map>

/**
 */
namespace Sudoku {

    typedef enum { ROW_MAJOR, BLOCK_COLS, GenPath_MAX = BLOCK_COLS, } GenPath;
    const std::array<std::string, 2> GenPath_Names = {
        "rowmajor",
        "blockcol",
    };
    typedef enum { HELP, QUIT, SOLVE, RUN_SINGLE, RUN_MULTIPLE, SET_GENPATH, } Command;
    const std::map<std::string, Command> COMMAND_MAP = {
        { "help",       HELP },
        { "quit",       QUIT },
        { "solve",      SOLVE },
        { "",           RUN_SINGLE },
        { "trials",     RUN_MULTIPLE },
        { "genpath",    SET_GENPATH },
    };
    const std::string HELP_MESSAGE = "\nCOMMAND MENU:"
        "\n- help           print this help menu."
        "\n- quit           terminate this program."
        "\n- solve <file>   solve the puzzle in <file>."
        "\n- {enter}        generate a single solution."
        "\n- trials <n>     generate <n> solutions."
        "\n- genpath        cycle generator traversal path."
        ;
    const std::string REPL_PROMPT = "\n> ";

    /**
     * About this enum template argument + conditional types approach:
     * (My learnings, resulting design choices, and their rationales)
     * 
     * First of all, the primary goal / benefit of this additional
     * complexity is that it optimizes on space usage: Smaller-order
     * grids will require less space for class members. I believe that
     * this will contribute to improved usage of the processor cache.
     * 
     * As I learned what design choices could be made, I kept in mind
     * these things:
     * 1. I want there to be well defined constraints on what values
     * can be provided as the template argument for order (reasonable
     * values are in the range [2,5]).
     * 2. Even though my REPL needs all the template expansions, I want
     * any caller code to be able to choose to omit certain expansions.
     * 3. I want to keep the conditional sizing typedefs internalized
     * to my code so there is no chance that it can be used incorrectly.
     * 
     * Keeping in mind the main goal (space optimization), let's see
     * some alternative designs / implementations that could achieve
     * the goal, but would violate my wishes:
     * 
     * 1. Make the sizing typedefs all template arguments. This clearly
     * violates #3, and does nothing to help #1. I could partially fix
     * #3 and #1 by providing named bundles with correct parameters via:
     *    1'. Partial template specification in the cpp file (violates #2).
     *    1''. Using alias templates in the hpp or cpp file.
     * 2. Take an unsigned short as a template argument for the grid's
     * order, and use conditional types. A small advantage here is that
     * I can condition typedefs on ranges of the order argument instead
     * of specifiers for each value. Use static assertions for valid range.
     * 3. Like the above solution, but uses enums (I didn't know about
     * static casts when I chose this solution - I will switch to #2 if
     * I find need to).
     */
    typedef enum { ORD_2 = 2, ORD_3 = 3, ORD_4 = 4, ORD_5 = 5, ORD_DEFAULT = ORD_4, } Order;
    const std::array<Order, 4> OrderVec = { ORD_2, ORD_3, ORD_4, ORD_5, };

    // Very large number container.
    typedef unsigned long long opcount_t;
    constexpr unsigned int TRIALS_NUM_BINS = 20;

    /**
     * 
     */
    template <Order O>
    class Solver {

    public:
        // mask width `order^2` bits.
        // order:  2   3   4   5   6   7   8   9  10  11
        // width:  4   9  16  25  36  49  64  81 100 121
        // round:  8  16  16  32  64  64  64 128 128 128
        typedef
            typename std::conditional_t<(O < 3), std::uint8_t,
            typename std::conditional_t<(O < 5), std::uint16_t,
            typename std::conditional_t<(O < 6), std::uint32_t,
            typename std::conditional_t<(O < 9), unsigned long,
            unsigned long long
        >>>> occmask_t;

        // uint range [0, order].
        typedef std::uint8_t order_t;

        // uint range [0, order^2].
        typedef
            typename std::conditional_t<(O <  4), std::uint8_t,
            typename std::conditional_t<(O < 16), std::uint16_t,
            std::uint32_t
        >> length_t;

        // uint range [0, order^4].
        // order:   2    3    4    5     6     7
        // area:   16   81  256  625  1296  2401
        // bytes:   2    2    3    4     4     4
        typedef
            typename std::conditional_t<(O < 4), std::uint8_t,
            typename std::conditional_t<(O < 8), std::uint16_t,
            std::uint32_t
        >> area_t;

        // uint range [0, order^2].
        typedef length_t value_t;

    public:
        /**
         * When clear, biasIndex is the parent Solver's length and value
         * is undefined.
         */
        class Tile {
            friend Solver;
        public:
            void clear(void) noexcept {
                biasIndex = length;
                //value = length;
            }
            // std::enable_if_t<(O == ORD_5), std::string> toString();
            // std::enable_if_t<(O < ORD_5), std::string> toString();
        protected:
            value_t biasIndex;
            value_t value; // undefined if clear.
        };


    public:
        static constexpr order_t    order   = O;
        static constexpr length_t   length  = O * O;
        static constexpr area_t     area    = O * O * O * O;

        explicit Solver(std::ostream&);

        // Return false if command is to exit the program:
        bool runCommand(std::string const& cmdLine);
        void runNew(void);
        void runMultiple(const unsigned long);
        void print(void) const;
        void printMessageBar(std::string const&, unsigned int, const char = '=') const;
        void printMessageBar(std::string const&, const char = '=') const;

    private:
        std::array<Tile, O*O*O*O> grid;
        std::array<occmask_t, O*O> rowSymbolOccMasks;
        std::array<occmask_t, O*O> colSymbolOccMasks;
        std::array<occmask_t, O*O> blkSymbolOccMasks;
        std::array<std::array<value_t, O*O+1>, O*O> rowBiases;
        std::array<area_t, O*O*O*O> traversalOrder;
        GenPath genPath;

        /**
         * https://www.desmos.com/calculator/8taqzelils
         * give up if number of operations performed exceeds this value.
         */
        static constexpr opcount_t giveupThreshold = ((const opcount_t[]){0,1,26,5200,5000000,500000000})[O];
        unsigned long totalGenCount;
        unsigned long successfulGenCount;

        std::ostream& os;
        const bool isPretty;
        static constexpr unsigned int statsWidth = (0.5 * length) + 3;
        const std::string gridHSepString;

        void clear(void);
        void seed(const bool printInfo);
        // Generates a random solution. Returns the number of operations or
        // zero if the give-up threshold was reached. Any previous seeds must
        // not make generating a solution impossible.
        opcount_t generateSolution(void);
        // Returns the tile at index.
        Tile const& setNextValid(const area_t index);
        length_t tileNumNonCandidates(const area_t) const noexcept __attribute__((const));
        void setGenPath(const GenPath) noexcept;

        // Inline functions:
        static bool isClear(Tile const& t) noexcept __attribute__((const)) { return t.biasIndex == length; }
        static length_t getRow(const area_t index) noexcept __attribute__((const)) { return index / length; }
        static length_t getCol(const area_t index) noexcept __attribute__((const)) { return index % length; }
        static length_t getBlk(const area_t index) noexcept __attribute__((const)) { return getBlk(getRow(index), getCol(index)); }
        static length_t getBlk(const length_t row, const length_t col) noexcept __attribute__((const)) {
            return ((row / order) * order) + (col / order);
        }

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