#ifndef HPP_SUDOKU_SOLVER
#define HPP_SUDOKU_SOLVER

#include "../util/ansi.hpp"
#include "./enum.hpp"
#include "./size.hpp"

#include <iostream>
#include <random>
#include <mutex>
#include <array>
#include <bitset>


namespace Sudoku { template <Sudoku::Order O> class Solver; }
template <Sudoku::Order O> std::ostream& operator<<(std::ostream&, Sudoku::Solver<O> const&);


namespace Sudoku {

    /**
     * 
     */
    template <Order O>
    class Solver {
    public:
        static constexpr bool   CBT = BUILDFLAG_CBT;
        static constexpr GUM::E GUM = BUILDFLAG_GUM;
        static_assert((1 < O) && (O <= MAX_REASONABLE_ORDER));
        static_assert((GUM == GUM::E::BACKTRACKS) ? CBT : true);
        using occmask_t     = typename Size<O>::occmask_t   ;
        using order_t       = typename Size<O>::order_t     ;
        using length_t      = typename Size<O>::length_t    ;
        using area_t        = typename Size<O>::area_t      ;
        using value_t       = typename Size<O>::value_t     ;
        using backtrack_t   = typename Size<O>::backtrack_t ;

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
            friend std::ostream& operator<<(std::ostream& out, Tile const& t) noexcept {
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
                            ? out << static_cast<unsigned>(t.value)
                            : out << static_cast<char>('a' + t.value - 10);
                    }
                }
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

        Solver(void) = delete;
        explicit Solver(std::ostream&);

        // Prints to std::cout and the output file if it exists.
        void print(void) const;
        // This took me so long to find out even what the problem was called.
        // My understanding is that all this line does is expose private members
        // to the non-member operator at the top-level namespace.
        // https://en.cppreference.com/w/cpp/language/friend#Template_friend_operators
        friend std::ostream& operator<< <O>(std::ostream&, Solver const& s);
        void printMessageBar(std::string const&, unsigned int, char = '=') const;
        void printMessageBar(std::string const&, char = '=') const;

        static constexpr opcount_t GIVEUP_THRESHOLD = Sudoku::Size<O>::template GIVEUP_THRESHOLD<GUM>;

        [[gnu::cold, gnu::pure]]
        GenPath::E getGenPath(void) const noexcept { return genPath; }
        [[gnu::cold]] GenPath::E setGenPath(GenPath::E, bool force = false) noexcept;
        [[gnu::cold]] GenPath::E setGenPath(std::string const&) noexcept;

        [[gnu::cold]] backtrack_t getMaxBacktrackCount(void) const noexcept { return maxBacktrackCount; }

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
        std::bitset<area> isTileForGiven;

        // These fields are used to continue the solution generator from
        // wherever it last left off.
        area_t prevGenTvsIndex;

        std::array<backtrack_t, (CBT?area:1)> backtrackCounts;
        backtrack_t maxBacktrackCount;
        void printShadedBacktrackStat(unsigned count) const;

    public:
        std::ostream& os;
        const bool isPretty;
        static constexpr unsigned STATS_WIDTH = (0.4 * length) + 4;
        static const std::string blkRowSepString;

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
        [[gnu::const]] static constexpr length_t getRow(const area_t index) noexcept { return index / length; }
        [[gnu::const]] static constexpr length_t getCol(const area_t index) noexcept { return index % length; }
        [[gnu::const]] static constexpr length_t getBlk(const area_t index) noexcept { return getBlk(getRow(index), getCol(index)); }
        [[gnu::const]] static constexpr length_t getBlk(const length_t row, const length_t col) noexcept {
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

} // End of Sudoku namespace

#endif