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
		using ord1_t  = typename size<O>::ord1_t;
		using ord2_t  = typename size<O>::ord2_t;
		using ord4_t  = typename size<O>::ord4_t;
		using value_t = typename size<O>::value_t;

		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;

		static constexpr opcount_t GIVEUP_THRESHOLD = gen_size<O>::GIVEUP_THRESHOLD;

		/**
		 * CLARITY:
		 * When clear, `this->value` is the grid's length.
		 *
		 * BIASINDEX:
		 * `this->try_progress` is the next value to try (such that valid outcomes
		 * are never skipped and the traversal never loops upon itself) if the
		 * try_progress pointing to my current value fails. If there is nothing left
		 * to try, this is set to the grid's length, indicating that the next thing
		 * to try is via backtracking.
		 */
		class Tile final {
			friend class Generator<O>;
		public:
			void clear(void) noexcept {
				try_progress = 0u;
				value = O2;
			}
			[[gnu::pure]] bool is_clear(void) const noexcept {
				return value == O2;
			}
			friend std::ostream& operator<<(std::ostream& out, Tile const& t) noexcept {
				// TODO [bug] Handle different blank formats.
				static_assert(O1 <= 6, "I haven't yet decided how to translate for orders > 6.");
				if (__builtin_expect(t.is_clear(), false)) {
					return out << ' ';
				} else {
					if constexpr (O1 < 4) {
						return out << (int)t.value;
					} else if constexpr (O1 == 5) {
						return out << static_cast<char>('a' + t.value);
					} else {
						return (t.value < 10)
							? out << static_cast<unsigned>(t.value)
							: out << static_cast<char>('a' + t.value - 10);
					}
				}
			}
		protected:
			value_t try_progress;
			value_t value; // undefined if clear.
		};

	// ========================
	public:
		Generator(void) = delete;
		explicit Generator(std::ostream&);
		[[gnu::cold]] void copy_settings_from(Generator const&);

		// Prints to std::cout and the output file if it exists.
		void print(void) const;
		void print_simple(void) const; // No newlines included.
		void print_msg_bar(std::string const&, unsigned int, char = '=') const;
		void print_msg_bar(std::string const&, char = '=') const;

		[[gnu::cold, gnu::pure]]
		GENPATH_STORAGE_MOD Path::E get_path_kind(void) noexcept { return path_kind; }
		// Setters return the old value of the generator path.
		[[gnu::cold]] GENPATH_STORAGE_MOD Path::E set_path_kind(Path::E, bool force = false) noexcept;
		[[gnu::cold]] GENPATH_STORAGE_MOD Path::E set_path_kind(std::string const&) noexcept;

		[[gnu::cold]] backtrack_t get_most_backtracks(void) const noexcept { return most_backtracks; }

	// ========================
	private:
		std::array<std::array<value_t, O2>, O2> val_try_order;
		std::array<Tile, O4> grid;
		std::array<occmask_t, O2> row_has;
		std::array<occmask_t, O2> col_has;
		std::array<occmask_t, O2> blk_has;

		/**
		 * (See Generator constructor) I've found that the choice of
		 * path_kind can make around a 2x difference in processor time,
		 * and also a visible difference in the distribution of the
		 * number of operations.
		 */
		[[gnu::cold]] GENPATH_STORAGE_MOD Path::E initialize_path(void) noexcept;
		GENPATH_STORAGE_MOD Path::E path_kind;
		GENPATH_STORAGE_MOD std::array<ord4_t, O4> path;

		std::array<backtrack_t, O4> backtrack_counts;
		backtrack_t most_backtracks;
		void print_shaded_backtrack_stat(backtrack_t) const;

	public:
		std::ostream& os;
		const bool is_pretty;
		static constexpr unsigned STATS_WIDTH = (0.4 * O2) + 4;
		static const std::string blk_row_sep_str;

	public:
		// Generates a random solution. Returns the number of operations
		// performed. If exit_status is not set to Exhausted, then an
		// immediate call to this method will continue the previous
		// solution-generating-run from where it left off.
		[[gnu::hot]] void generate(bool _continue = false);
		struct PrevGen final {
		friend class Generator;
		private:
			ExitStatus exit_status = ExitStatus::Exhausted;
			ord4_t     progress    = 0;
			opcount_t  op_count    = 0;
		public:
			ExitStatus get_exist_status(void) const { return exit_status; }
			opcount_t  get_op_count(void) const { return op_count; }
		}; PrevGen prev_gen;
	private:
		void clear(void);
		[[gnu::hot]] PathDirection set_next_valid(const ord4_t);
	};

	const std::string GRID_SEP = "  ";

	#undef GENPATH_STORAGE_MOD
}

#endif