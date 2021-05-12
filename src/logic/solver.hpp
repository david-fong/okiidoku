#ifndef HPP_SUDOKU_SOLVER
#define HPP_SUDOKU_SOLVER

#include "./grid.hpp"
#include "../util/ansi.hpp"
#include "./enum.hpp"
#include "./size.hpp"

#include <iostream>
#include <random>
#include <mutex>
#include <array>
#include <bitset>


// https://en.cppreference.com/w/cpp/language/friend#Template_friend_operators
// https://web.mst.edu/~nmjxv3/articles/templates.html
namespace Sudoku::Solver { template <Sudoku::Order O> class Solver; }
template <Sudoku::Order O> std::ostream& operator<<(std::ostream&, Sudoku::Solver::Solver<O> const&);


namespace Sudoku::Solver {

	// Note: These are #undef-ed at the end of this file.
	#if SOLVER_THREADS_SHARE_GENPATH
	#define GENPATH_STORAGE_MOD static
	#else
	#define GENPATH_STORAGE_MOD
	#endif

	/**
	 */
	template <Order O>
	class Solver final : public Sudoku::Grid::Grid<O> {
		friend std::ostream& operator<< <O>(std::ostream&, Solver const& s);
	public:
		using backtrack_t = typename SolverSize<O>::backtrack_t;
		using occmask_t = typename Size<O>::occmask_t;
		using order_t  = typename Size<O>::order_t;
		using length_t = typename Size<O>::length_t;
		using area_t   = typename Size<O>::area_t;
		using value_t  = typename Size<O>::value_t;

	public:
		static constexpr opcount_t GIVEUP_THRESHOLD = SolverSize<O>::GIVEUP_THRESHOLD;

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
		 */
		class Tile final {
			friend class Solver<O>;
		public:
			void clear(void) noexcept {
				biasIndex = 0u;
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
		protected:
			value_t biasIndex;
			value_t value; // undefined if clear.
		};

	// ========================
	// PUBLIC MEMBERS
	// ========================
	public:
		static constexpr order_t  order  = O;
		static constexpr length_t length = O*O;
		static constexpr area_t   area   = O*O*O*O;
		Solver(void) = delete;
		explicit Solver(std::ostream&);
		[[gnu::cold]] void copySettingsFrom(Solver const&);

		// Prints to std::cout and the output file if it exists.
		void print(void) const;
		void printSimple(void) const; // No newlines included.
		void printMessageBar(std::string const&, unsigned int, char = '=') const;
		void printMessageBar(std::string const&, char = '=') const;

		[[gnu::cold, gnu::pure]]
		GENPATH_STORAGE_MOD GenPath::E getGenPath(void) noexcept { return genPath; }
		// Setters return the old value of the generator path.
		[[gnu::cold]] GENPATH_STORAGE_MOD GenPath::E setGenPath(GenPath::E, bool force = false) noexcept;
		[[gnu::cold]] GENPATH_STORAGE_MOD GenPath::E setGenPath(std::string const&) noexcept;

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
		[[gnu::cold]] GENPATH_STORAGE_MOD GenPath::E initializeGenPath(void) noexcept;
		GENPATH_STORAGE_MOD GenPath::E genPath;
		GENPATH_STORAGE_MOD std::array<area_t, area> traversalOrder;

		std::array<backtrack_t, area> backtrackCounts;
		backtrack_t maxBacktrackCount;
		void printShadedBacktrackStat(backtrack_t) const;

	public:
		std::ostream& os;
		const bool isPretty;
		static constexpr unsigned STATS_WIDTH = (0.4 * length) + 4;
		static const std::string blkRowSepString;

	public:
		// Generates a random solution. Returns the number of operations
		// performed. If exitStatus is not set to IMPOSSIBLE, then an
		// immediate call to this method will continue the previous
		// solution-generating-run from where it left off.
		[[gnu::hot]] void generateSolution(bool contPrev = false);
		struct PrevGen final {
		  friend class Solver;
		  private:
			ExitStatus exitStatus = ExitStatus::IMPOSSIBLE;
			area_t     tvsIndex   = 0;
			opcount_t  opCount    = 0;
		  public:
			ExitStatus getExitStatus(void) const { return exitStatus; }
			opcount_t  getOpCount(void) const { return opCount; }
		}; PrevGen prevGen;
	private:
		void clear(void);
		[[gnu::hot]] TvsDirection setNextValid(const area_t);
	}; // class Solver


	const std::string GRID_SEP = "  ";

	#undef GENPATH_STORAGE_MOD

} // namespace Sudoku::Solver

#endif