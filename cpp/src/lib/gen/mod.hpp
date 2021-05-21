#ifndef HPP_SOLVENT_LIB_GEN
#define HPP_SOLVENT_LIB_GEN

#include "../../util/ansi.hpp"
#include "../size.hpp"
#include "../grid.hpp"
#include "./path.hpp"

#include <array>

// https://en.cppreference.com/w/cpp/language/friend#Template_friend_operators
// https://web.mst.edu/~nmjxv3/articles/templates.html
namespace solvent::lib::gen { template <solvent::Order O> class Generator; }
template <solvent::Order O> std::ostream& operator<<(std::ostream&, solvent::lib::gen::Generator<O> const&);


namespace solvent::lib::gen {

	// Note: These are #undef-ed at the end of this file.
	#if SOLVER_THREADS_SHARE_GENPATH
	#define GENPATH_STORAGE_MOD static
	#else
	#define GENPATH_STORAGE_MOD
	#endif

	/**
	 */
	template <Order O>
	class Generator final : public Grid<O> {
		friend std::ostream& operator<< <O>(std::ostream&, Generator const& s);
	public:
		using backtrack_t = typename gen_size<O>::backtrack_t;
		using occmask_t = typename size<O>::occmask_t;
		using order_t   = typename size<O>::order_t;
		using length_t  = typename size<O>::length_t;
		using area_t    = typename size<O>::area_t;
		using value_t   = typename size<O>::value_t;

		static constexpr order_t  O1 = O;
		static constexpr length_t O2 = O*O;
		static constexpr area_t   O4 = O*O*O*O;

		static constexpr opcount_t GIVEUP_THRESHOLD = gen_size<O>::GIVEUP_THRESHOLD;

		/**
		 * CLARITY:
		 * When clear, `this->value` is the grid's length.
		 *
		 * BIASINDEX:
		 * `this->biasIndex` is the next biasIndex to try (such that valid outcomes
		 * are never skipped and the traversal never loops upon itself) if the
		 * biasIndex pointing to my current value fails. If there is nothing left
		 * to try, this is set to the grid's length, indicating that the next thing
		 * to try is via backtracking.
		 */
		class Tile final {
			friend class Generator<O>;
		public:
			void clear(void) noexcept {
				biasIndex = 0u;
				value = O2;
			}
			[[gnu::pure]] bool isClear(void) const noexcept {
				return value == O2;
			}
		protected:
			value_t biasIndex;
			value_t value; // undefined if clear.
		};

	// ========================
	public:
		Generator(void) = delete;
		explicit Generator(std::ostream&);
		[[gnu::cold]] void copy_settings_from(Generator const&);

		// Prints to std::cout and the output file if it exists.
		void print(void) const;
		void printSimple(void) const; // No newlines included.
		void printMessageBar(std::string const&, unsigned int, char = '=') const;
		void printMessageBar(std::string const&, char = '=') const;

		[[gnu::cold, gnu::pure]]
		GENPATH_STORAGE_MOD Path::E getGenPath(void) noexcept { return genPath; }
		// Setters return the old value of the generator path.
		[[gnu::cold]] GENPATH_STORAGE_MOD Path::E setGenPath(Path::E, bool force = false) noexcept;
		[[gnu::cold]] GENPATH_STORAGE_MOD Path::E setGenPath(std::string const&) noexcept;

		[[gnu::cold]] backtrack_t getMaxBacktrackCount(void) const noexcept { return max_backtracks; }

	// ========================
	private:
		std::array<Tile, O4> grid;
		std::array<occmask_t, O2> rowSymbolOccMasks;
		std::array<occmask_t, O2> colSymbolOccMasks;
		std::array<occmask_t, O2> blkSymbolOccMasks;
		std::array<std::array<value_t, O2>, O2> rowBiases;

		/**
		 * (See Generator constructor) I've found that the choice of
		 * genPath can make around a 2x difference in processor time,
		 * and also a visible difference in the distribution of the
		 * number of operations.
		 */
		[[gnu::cold]] GENPATH_STORAGE_MOD Path::E initializeGenPath(void) noexcept;
		GENPATH_STORAGE_MOD Path::E genPath;
		GENPATH_STORAGE_MOD std::array<area_t, O4> traversalOrder;

		std::array<backtrack_t, O4> backtrackCounts;
		backtrack_t max_backtracks;
		void printShadedBacktrackStat(backtrack_t) const;

	public:
		std::ostream& os;
		const bool isPretty;
		static constexpr unsigned STATS_WIDTH = (0.4 * O2) + 4;
		static const std::string blkRowSepString;

	public:
		// Generates a random solution. Returns the number of operations
		// performed. If exitStatus is not set to Exhausted, then an
		// immediate call to this method will continue the previous
		// solution-generating-run from where it left off.
		[[gnu::hot]] void generateSolution(bool contPrev = false);
		struct PrevGen final {
		friend class Generator;
		private:
			ExitStatus exitStatus = ExitStatus::Exhausted;
			area_t     tvsIndex   = 0;
			opcount_t  opCount    = 0;
		public:
			ExitStatus getExitStatus(void) const { return exitStatus; }
			opcount_t  getOpCount(void) const { return opCount; }
		}; PrevGen prev_gen;
	private:
		void clear(void);
		[[gnu::hot]] PathDirection setNextValid(const area_t);
	};

	const std::string GRID_SEP = "  ";

	#undef GENPATH_STORAGE_MOD
}

#endif