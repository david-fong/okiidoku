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

    // Container for a very large number.
    // See Solver::GIVEUP_THRESHOLD for more discussion on the average
    // number of operations taken to generate a solution by grid-order.
    typedef unsigned long long opcount_t;
    constexpr unsigned int TRIALS_NUM_BINS = 20;

    /**
     * The primary goal of this added complexity is to make effective
     * use of space, which should help improve cache performance.
     * 
     * As I explored my implementation options, I set these goals:
     * 1. Grid order template parameter is bounded within reason.
     * 2. Caller code can choose which template expansions to compile.
     * 3. Conditional sizing typedefs are kept internal.
     * 
     * Constructs I could have used in less favourable implementations
     * included: partial template specification, alias templating,
     * using an enum type for valid grid orders.
     */
    typedef uint8_t Order;
    constexpr Order MAX_REASONABLE_ORDER = 20;

    /**
     * 
     */
    template <Order O>
    class Solver {
        static_assert(1 < O && O <= MAX_REASONABLE_ORDER);

    /**
     * TYPEDEFS
     */
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

    /**
     * HELPER CLASS
     */
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
        static constexpr length_t   length  = O*O;
        static constexpr area_t     area    = O*O*O*O;

        explicit Solver(std::ostream&);

        // Return false if command is to exit the program:
        bool runCommand(std::string const& cmdLine);
        void runNew(void);
        void runMultiple(const unsigned long);
        void print(void) const;
        void printMessageBar(std::string const&, unsigned int, const char = '=') const;
        void printMessageBar(std::string const&, const char = '=') const;

    private:
        std::array<Tile, area> grid;
        std::array<occmask_t, length> rowSymbolOccMasks;
        std::array<occmask_t, length> colSymbolOccMasks;
        std::array<occmask_t, length> blkSymbolOccMasks;
        std::array<std::array<value_t, length+1>, length> rowBiases;

        // Interesting! Smaller-order grids perform better with ROW_MAJOR as genPath.
        static constexpr GenPath DEFAULT_GENPATH = (O < 4) ? ROW_MAJOR : BLOCK_COLS;
        GenPath genPath;
        std::array<area_t, area> traversalOrder;

        /**
         * Give up if number of operations performed exceeds this value.
         * Measured stats: https://www.desmos.com/calculator/8taqzelils
         * 
         * (See Solver::DEFAULT_GENPATH) I've found that the choice of
         * genPath can make around a 2x difference in processor time,
         * and also a visible difference in the distribution of the
         * number of operations.
         * 
         * How I chose these values (for each grid-order):
         * - Order 2: No giveups.
         * - Order 3: No giveups.
         * - Order 4: Giveup ratio is less than 1%.
         * - Order 5: More complicated: I ran 100 trials with a threshold
         *   of 500M. The giveup ratio was ~53%, and the cheaper ~79% of
         *   _successes_ took less than 225M. If I use that as the new
         *   threshold, I should suffer a ~64% giveup ratio, but spend
         *   fewer operations on generations that are likely to be given
         *   up anyway. Say we approximate the giveup threshold to be
         *   the average number of operations per trial (actual value
         *   is less). Then the throughput/efficiency can be calculated
         *   as the number of successes per trial over the number of
         *   operations per trial. With thresh = 500M, efficiency =
         *   9.4e-10, with 225M, 1.64e-9, and with 25M, 5.2e-9!
         */
        static constexpr opcount_t GIVEUP_THRESHOLD = ((const opcount_t[]){0,1,25,2'500,5'000'000,25'000'000})[O];
        unsigned long totalGenCount;
        unsigned long successfulGenCount;
        bool doCountBacktracks;
        std::array<unsigned, area> backtrackCounts; // Ordered according to genPath.

        std::ostream& os;
        const bool isPretty;
        static constexpr unsigned int statsWidth = (0.5 * length) + 3;
        const std::string gridHSepString;

        void clear(void);
        void seed(const bool printInfo);
        // Generates a random solution. Returns the number of operations or
        // zero if the give-up threshold was reached or if any previous seeds
        // made generating a solution impossible.
        opcount_t generateSolution(void);
        // Returns the tile that was operated on.
        Tile const& setNextValid(const area_t);
        [[gnu::const]] length_t tileNumNonCandidates(const area_t) const noexcept;
        void setGenPath(const GenPath) noexcept;

        // Inline functions:
        [[gnu::const]] static bool isClear(Tile const& t) noexcept { return t.biasIndex == length; }
        [[gnu::const]] static length_t getRow(const area_t index) noexcept { return index / length; }
        [[gnu::const]] static length_t getCol(const area_t index) noexcept { return index % length; }
        [[gnu::const]] static length_t getBlk(const area_t index) noexcept { return getBlk(getRow(index), getCol(index)); }
        [[gnu::const]] static length_t getBlk(const length_t row, const length_t col) noexcept {
            return ((row / order) * order) + (col / order);
        }

    private:
        static int MY_RANDOM (const int i) { return std::rand() % i; }
        struct MyNumpunct : std::numpunct<char> {
            std::string do_grouping() const {
                return "\03";
            }
        };
    };

} // End of Sudoku namespace

#endif