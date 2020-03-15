#ifndef HPP_SUDOKU_SOLVER
#define HPP_SUDOKU_SOLVER

#include "../util/ansi.hpp"
#include "./enum.hpp"

#include <locale>   // numpunct
#include <iostream>
#include <array>


/**
 * 
 * DIM:TEXT:
 * If the macro `USE_ANSI_ESC` is set truthy before the first link to
 * the implementation files for classes in this namespace, then some
 * harsh, non-essential text will be printed with ansi escape codes to
 * dim the color.
 * 
 * FAST:IO:
 * Note: My implementation does not use C-style IO, so it is safe for
 * consumer code to make the following optimization:
 * ```cpp
 * std::ios_base::sync_with_stdio(false);
 * ```
 */
namespace Sudoku {

    // Container for a very large number.
    // See Solver::GIVEUP_THRESHOLD for more discussion on the average
    // number of operations taken to generate a solution by grid-order.
    typedef unsigned long long opcount_t;

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
     * 
     * Note that there is another similar typedef named `order_t`,
     * which is conditionally defined to be large enough for whatever
     * value is passed as a template parameter of this type (Order)
     * as a maximum value.
     */
    typedef uint8_t Order;
    constexpr Order MAX_REASONABLE_ORDER = 20;


    /**
     * 
     * 
     * An important note when working with this class: When printing
     * values of numeric types defined within this class, make sure to
     * cast any non-area types upward where they may conditionally be
     * a `uint8_t` for small grid-orders, or else it will be specially
     * interpreted as a char (I've already been bitten twice by this).
     * 
     * @param CBT - Whether or not to count backtrack statistics
     * @param GUM - Giveup method.
     */
    template <Order O, bool CBT, GUM::E GUM>
    class Solver {
        static_assert((1 < O) && (O <= MAX_REASONABLE_ORDER));
        static_assert((GUM == GUM::E::BACKTRACKS) ? CBT : true);

    // ========================
    // TYPEDEFS
    // ========================
    public:
        // mask width `order^2` bits.
        // order:  2   3   4   5   6   7   8   9  10  11
        // width:  4   9  16  25  36  49  64  81 100 121
        // round:  8  16  16  32  64  64  64 128 128 128
        typedef
            typename std::conditional_t<(O < 3), std::uint_fast8_t,
            typename std::conditional_t<(O < 5), std::uint_fast16_t,
            typename std::conditional_t<(O < 6), std::uint_fast32_t,
            typename std::conditional_t<(O < 9), unsigned long,
            unsigned long long
        >>>> occmask_t;

        // uint range [0, order].
        typedef std::uint8_t order_t;

        // uint range [0, order^2].
        typedef
            typename std::conditional_t<(O <  4), std::uint_fast8_t,
            typename std::conditional_t<(O < 16), std::uint_fast16_t,
            std::uint_fast32_t
        >> length_t;

        // uint range [0, order^4].
        // order:   2    3    4    5     6     7     8     9     10
        // area:   16   81  256  625  1296  2401  4096  6561  10000
        // bits:    5    7    9    9    11    12    17    17     18
        typedef
            typename std::conditional_t<(O <   4), std::uint8_t,
            typename std::conditional_t<(O <   8), std::uint16_t,
            typename std::conditional_t<(O < 256), std::uint32_t,
            unsigned long
        >>> area_t;

        // uint range [0, order^2].
        typedef length_t value_t;

    // =======================
    // HELPER CLASS
    // =======================
    public:
        /**
         * CLARITY:
         * When clear, `this->value` is the grid's length.
         * 
         * BIASINDEX:
         * `this->biasIndex` is the next biasIndex to try (such that
         * valid outcomes are never skipped and the traversal never
         * loops upon itself) if the biasIndex pointing to my current
         * value fails. If there is nothing left to try, this is set
         * to the grid's length, indicating that the next thing to try
         * is via backtracking.
         * 
         * GIVENS:
         * If solving a puzzle and this tile is for given information,
         * `biasIndex` should not be used, and `value` should not be
         * modified.
         */
        class Tile {
            friend class Solver;
        public:
            void clear(void) noexcept {
                biasIndex = 0;
                value = length;
            }
            [[gnu::pure]] bool isClear(void) const noexcept {
                return value == length;
            }
            friend std::ostream& operator<<(std::ostream& out, Tile const& t) {
                // TODO [bug] Handle different blank formats.
                static_assert(order <= 6, "I haven't yet decided how to translate for orders > 6.");
                if (__builtin_expect(t.isClear(), false)) {
                    return out << ' ';
                } else {
                    if constexpr (order < 4) {
                        return out << (int)t.value;
                    } else if constexpr (order == 5) {
                        return out << static_cast<char>('a' + t.value);
                    } else {
                        return (t.value < 10)
                            ? out << t.value
                            : out << static_cast<char>('a' + t.value - 10);
                    }
                }
                std::terminate();
            }
            // Does not check the validity of inputs.
            // Assumes that the puzzle-string blanks style uses <length>
            // for blanks. Accepts 'G' for order == 4 as `length`, and
            // 'A' for order == 6. Case sensitive. Always lowercase first
            // for order <= 6.
            [[gnu::const]] static constexpr value_t VALUE_FROM_CHAR(const char valueChar) noexcept {
                static_assert(order <= 6, "I haven't yet decided how to translate for orders > 6.");
                if constexpr (order < 4) {
                    return valueChar - '0';
                } else if constexpr (order == 5) {
                    return valueChar - 'a';
                } else {
                    if constexpr (order == 6)
                        if (valueChar == 'A') return 36;
                    return (valueChar <= '9')
                        ?  (valueChar  - '0')
                        :  (valueChar  - 'a' + 10);
                }
            }
        protected:
            value_t biasIndex;
            value_t value; // undefined if clear.
        };

    // ========================
    // PUBLIC MEMBERS
    // ========================
    public:
        static constexpr order_t    order   = O;
        static constexpr length_t   length  = O*O;
        static constexpr area_t     area    = O*O*O*O;

        explicit Solver(std::ostream&);

        void print(void) const;
        void printMessageBar(std::string const&, unsigned int, char = '=') const;
        void printMessageBar(std::string const&, char = '=') const;

        [[gnu::cold, gnu::pure]]
        GenPath::E getGenPath(void) const noexcept { return genPath; }
        [[gnu::cold]] GenPath::E setGenPath(GenPath::E, bool force = false) noexcept;
        [[gnu::cold]] GenPath::E setGenPath(std::string const&) noexcept;

    // ========================
    // PRIVATE MEMBERS
    // ========================
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
        GenPath::E genPath;
        std::array<area_t, area> traversalOrder;
        std::array<bool,   area> isTileForGiven;

        // These fields are used to continue the solution generator from
        // wherever it last left off.
        area_t prevGenTvsIndex;

    private:
        unsigned long long totalGenCount = 0;
        std::array<unsigned, (CBT?area:1)> backtrackCounts;
        opcount_t maxBacktrackCount;
        void printShadedBacktrackStat(unsigned count) const;
    public:
        [[gnu::cold]] unsigned long long getTotalGenCount(void) const noexcept { return totalGenCount; }
        [[gnu::cold]] opcount_t getMaxBacktrackCount(void) const noexcept { return maxBacktrackCount; }

    public:
        /**
         * Give up if the giveup condition variable meets this value.
         * Measured stats for operations: https://www.desmos.com/calculator/8taqzelils
         */
        static constexpr opcount_t GIVEUP_THRESHOLD
            = (GUM == GUM::E::OPERATIONS) ? ((const opcount_t[]){
                1, 2, 26, 2'000, 100'000, 30'000'000, 120'000'000'000,
                })[order]
            : (GUM == GUM::E::BACKTRACKS) ? ((const opcount_t[]){
                1, 1,  3,   150,  10'000,  2'200'000,  10'000'000'000,
                })[order]
            : [](){ throw "unhandled GUM case"; return ~0; }();
            // TODO [tune] update the above numbers. the current values for order-6 are just predictions.

    public:
        std::ostream& os;
        const bool isPretty;
        std::locale benchedLocale; // Used to swap in-and-out the thousands-commas.
        static constexpr unsigned STATS_WIDTH = (0.4 * length) + 4;
        const std::string blkRowSepString;

    public:
        // Returns whether the string could be loaded as a puzzle.
        // Does NOT check whether the givens follow the sudoku rules.
        bool loadPuzzleFromString(std::string const&);
        // Generates a random solution. Returns the number of operations
        // performed. If exitStatus is not set to IMPOSSIBLE, then an
        // immediate call to this method will continue the previous
        // solution-generating-run from where it left off.
        template <bool USE_PUZZLE = false>
        [[gnu::hot]] opcount_t generateSolution(SolverExitStatus& exitStatus, bool contPrev = false);
    private:
        void registerGivenValue(area_t index, value_t value);
        template <bool USE_PUZZLE>
        [[gnu::hot]] void clear(void);
        [[gnu::hot]] TvsDirection setNextValid(const area_t);

    // ========================
    // STATIC UTILITIES
    // ========================
    public:
        // Inline functions:
        [[gnu::const]] static length_t getRow(const area_t index) noexcept { return index / length; }
        [[gnu::const]] static length_t getCol(const area_t index) noexcept { return index % length; }
        [[gnu::const]] static length_t getBlk(const area_t index) noexcept { return getBlk(getRow(index), getCol(index)); }
        [[gnu::const]] static length_t getBlk(const length_t row, const length_t col) noexcept {
            return ((row / order) * order) + (col / order);
        }
        [[gnu::const]] static length_t occmask_popcount(occmask_t occmask) noexcept {
            if constexpr (O < 6) {
                return __builtin_popcount(occmask);
            } else if constexpr (O < 9) {
                return __builtin_popcountl(occmask);
            } else {
                return __builtin_popcountll(occmask);
            }
        }
        [[gnu::const]] static length_t occmask_ctz(occmask_t occmask) noexcept {
            if constexpr (O < 6) {
                return __builtin_ctz(occmask);
            } else if constexpr (O < 9) {
                return __builtin_ctzl(occmask);
            } else {
                return __builtin_ctzll(occmask);
            }
        }
    }; // End of Solver class.


    const std::string GRID_SEP = "  ";
    int MY_RANDOM(const int i) { return std::rand() % i; }
    struct MyNumpunct : std::numpunct<char> {
        std::string do_grouping() const {
            return "\03";
        }
    };

} // End of Sudoku namespace

#endif