#ifndef _GRID_H_
#define _GRID_H_

#include <array>
#include <map>

/**
 * 
 * Note: My implementation does not use C-style IO, so it is safe for
 * consumer code to make the following optimization:
 * ```cplusplus
 * std::ios_base::sync_with_stdio(false);
 * ```
 */
namespace Sudoku {

    typedef enum {
        CMD_HELP,
        CMD_QUIT,
        CMD_SOLVE,
        CMD_RUN_SINGLE,
        CMD_RUN_MULTIPLE,
        CMD_SET_GENPATH,
        CMD_DO_BACKTRACK_COUNT,
    } Command;
    const std::map<std::string, Command> COMMAND_MAP = {
        { "help",       CMD_HELP },
        { "quit",       CMD_QUIT },
        { "solve",      CMD_SOLVE },
        { "",           CMD_RUN_SINGLE },
        { "trials",     CMD_RUN_MULTIPLE },
        { "genpath",    CMD_SET_GENPATH },
    };
    const std::string HELP_MESSAGE = "\nCOMMAND MENU:"
        "\n- help           print this help menu"
        "\n- quit           terminate this program"
        "\n- solve <file>   solve the puzzle in <file>"
        "\n- {enter}        generate a single solution"
        "\n- trials <n>     generate <n> solutions"
        "\n- genpath        cycle generator traversal path"
        ;
    const std::string REPL_PROMPT = "\n$ ";
    static const std::string GRID_SEP = "  ";

    typedef enum { ROW_MAJOR, BLOCK_COLS, GenPath_MAX = BLOCK_COLS, } GenPath;
    const std::array<std::string, 2> GenPath_Names = {
        "rowmajor",
        "blockcol",
    };

    // Container for a very large number.
    // See Solver::GIVEUP_THRESHOLD for more discussion on the average
    // number of operations taken to generate a solution by grid-order.
    typedef unsigned long long opcount_t;
    typedef unsigned long trials_t;
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

    enum TraversalDirection : bool {
        BACK = false, FORWARD = true,
    };

    /**
     * 
     * 
     * An important note when working with this class: When printing
     * values of numeric types defined within this class, make sure to
     * cast any non-area types upward where they may conditionally be
     * a `uint8_t` for small grid-orders, or else it will be specially
     * interpreted as a char (I've already been bitten twice by this).
     */
    template <Order O, bool CBT>
    class Solver {
        static_assert((1 < O) && (O <= MAX_REASONABLE_ORDER));

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
        // TODO: can we just be lazy and say biasIndex is undefined if
        // solving a puzzle and this tile's value is a given / hint?
        class Tile {
            friend Solver;
        public:
            void clear(void) noexcept {
                biasIndex = length;
                value = length;
            }
            [[gnu::const]] bool isClear(void) const noexcept {
                return value == length;
            }
            friend std::ostream& operator<<(std::ostream& out, Tile const& t) {
                if (__builtin_expect(t.isClear(), false)) {
                    return out << ' ';
                } else {
                    if constexpr (order < 5) {
                        return out << (uint16_t)t.value;
                    } else {
                        return out << (char)('a' + t.value);
                    }
                }
            }
        protected:
            value_t biasIndex;
            value_t value; // undefined if clear.
        };

    /**
     * PUBLIC MEMBERS
     */
    public:
        static constexpr order_t    order   = O;
        static constexpr length_t   length  = O*O;
        static constexpr area_t     area    = O*O*O*O;

        explicit Solver(std::ostream&);

        // Return false if command is to exit the program:
        bool runCommand(std::string const& cmdLine);
        void solvePuzzlesFromFile(std::ifstream&);
        void runNew(void);
        void runMultiple(const unsigned long);

        void printTrialsWorkDistribution(const trials_t,
            std::array<trials_t, TRIALS_NUM_BINS> const&,
            std::array<double,   TRIALS_NUM_BINS> const&);
        void print(void) const;

    /**
     * PRIVATE MEMBERS
     */
    private:
        std::array<Tile, area> grid;
        std::array<occmask_t, length> rowSymbolOccMasks;
        std::array<occmask_t, length> colSymbolOccMasks;
        std::array<occmask_t, length> blkSymbolOccMasks;
        std::array<std::array<value_t, length>, length> rowBiases;

        /**
         * (See Solver constructor) I've found that the choice of
         * genPath can make around a 2x difference in processor time,
         * and also a visible difference in the distribution of the
         * number of operations.
         */
        GenPath genPath;
        std::array<area_t, area> traversalOrder;
        [[gnu::cold]] void setGenPath(const GenPath) noexcept;

        /**
         * Give up if number of operations performed exceeds this value.
         * Measured stats: https://www.desmos.com/calculator/8taqzelils
         */
        static constexpr opcount_t GIVEUP_THRESHOLD = ((const opcount_t[]){
            0, 1, 25, 2'000, 2'500'000, 30'000'000,
        })[order];
        unsigned long totalGenCount;
        unsigned long successfulGenCount;
        std::array<unsigned, (CBT ? area : 1)> backtrackCounts; // Same ordering as this->grid.
        void printBacktrackStat(const area_t index, unsigned const& worstCount) const;

        std::ostream& os;
        const bool isPretty;
        std::locale benchedLocale; // Used to swap in-and-out the thousands-commas.
        static constexpr unsigned statsWidth = (0.4 * length) + 4;
        const std::string blkRowSepString;

        void printMessageBar(std::string const&, unsigned int, const char = '=') const;
        void printMessageBar(std::string const&, const char = '=') const;

        void clear(void);
        // Generates a random solution. Returns the number of operations or
        // zero if the give-up threshold was reached or if any previous seeds
        // made generating a solution impossible.
        template <bool USE_PUZZLE>
        [[gnu::hot]] opcount_t generateSolution(void);
        [[gnu::hot]] TraversalDirection setNextValid(const area_t);
        length_t tileNumNonCandidates(const area_t) const noexcept;

        // Inline functions:
        [[gnu::const]] static length_t getRow(const area_t index) noexcept { return index / length; }
        [[gnu::const]] static length_t getCol(const area_t index) noexcept { return index % length; }
        [[gnu::const]] static length_t getBlk(const area_t index) noexcept { return getBlk(getRow(index), getCol(index)); }
        [[gnu::const]] static length_t getBlk(const length_t row, const length_t col) noexcept {
            return ((row / order) * order) + (col / order);
        }

    /**
     * PRIVATE STATIC
     */
    private:
        static int MY_RANDOM (const int i) { return std::rand() % i; }
        struct MyNumpunct : std::numpunct<char> {
            std::string do_grouping() const {
                return "\03";
            }
        };
        static unsigned int GET_TERMINAL_COLUMNS(const unsigned int fallback) noexcept {
            char const*const envVar = std::getenv("COLUMNS");
            return (envVar != NULL) ? std::stoul(envVar) : fallback;
        }
    };

} // End of Sudoku namespace

#endif