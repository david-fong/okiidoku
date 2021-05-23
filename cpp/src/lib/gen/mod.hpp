#ifndef HPP_SOLVENT_LIB_GEN
#define HPP_SOLVENT_LIB_GEN

#include "../size.hpp"
#include "../grid.hpp"
#include "./path.hpp"

#include <array>
#include <string>
#include <ostream>

// https://en.cppreference.com/w/cpp/language/friend#Template_friend_operators
// https://web.mst.edu/~nmjxv3/articles/templates.html
namespace solvent::lib::gen { template<solvent::Order O> class Generator; }
template<solvent::Order O> std::ostream& operator<<(std::ostream&, solvent::lib::gen::Generator<O> const&);


namespace solvent::lib::gen {

	// Container for a very large number.
	// See Solver::GIVEUP_THRESHOLD for more discussion on the average
	// number of operations taken to generate a solution by grid-order.
	typedef unsigned long long opcount_t;

	/**
	 * Note: when printing, make sure to cast uint8_t to int.
	 */
	template<Order O>
	struct gen_size {
		public:
		// Note that this should always be smaller than opcount_t.
		typedef
			// Make sure this can fit `GIVEUP_THRESHOLD<Backtracks>`.
			//typename std::conditional_t<(O < 4), std::uint_fast8_t,
			typename std::conditional_t<(O < 5), std::uint_fast16_t,
			typename std::conditional_t<(O < 6), std::uint_fast32_t,
			std::uint64_t
		>> backtrack_t;

		/**
		 * Give up if the giveup condition variable meets this value.
		 * Measured stats for operations: https://www.desmos.com/calculator/8taqzelils
		 */
		static constexpr opcount_t GIVEUP_THRESHOLD = ((const opcount_t[]){
			// Note: Make sure entries of `backtrack_counts` can fit these.
			0,  1,  3,  150,  1'125,  560'000,  1'000'000'000,
		})[O];
		// TODO [tune] The current values for order-6 are just predictions.
	};

	struct GenResult final {
		unsigned int run_number = ~0;
		ExitStatus exit_status = ExitStatus::Exhausted;
		unsigned int progress = 0;
		opcount_t op_count = 0;
	};

	std::string shaded_backtrack_stat(backtrack_t);

	/**
	 */
	template<Order O>
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

		/** */
		class Tile final {
		 friend class Generator<O>;
		 public:
			void clear(void) noexcept {
				try_index = 0;
				value = O2;
			}
			[[gnu::pure]] bool is_clear(void) const noexcept {
				return value == O2;
			}
			friend std::ostream& operator<<(std::ostream& os, Tile const& t) noexcept {
				static_assert(O1 <= 6, "I haven't yet decided how to translate for orders > 6.");
				if (t.is_clear()) [[unlikely]] {
					return os << ' ';
				} else {
					if constexpr (O1 < 4) {
						return os << (int)t.value;
					} else if constexpr (O1 == 5) {
						return os << static_cast<char>('a' + t.value);
					} else {
						return (t.value < 10)
							? os << static_cast<unsigned>(t.value)
							: os << static_cast<char>('a' + t.value - 10);
					}
				}
			}
		 private:
			// `this->try_index` is the next value to try (such that valid outcomes
			// are never skipped and the traversal never loops upon itself) if the
			// try_index pointing to my current value fails. If there is nothing left
			// to try, this is set to the grid's length, indicating that the next thing
			// to try is backtracking.
			value_t try_index;
			value_t value;
		};

	// ========================
	 public:
		Generator(void) = delete;
		explicit Generator(std::ostream&);
		[[gnu::cold]] void copy_settings_from(Generator const&);

		std::ostream& os;
		const bool is_pretty;
		static constexpr unsigned STATS_WIDTH = (0.4 * O2) + 4;
		static const std::string blk_row_sep_str;

		// Prints to std::cout and the output file if it exists.
		void print(void) const;
		void print_simple(void) const; // No newlines included.
		void print_msg_bar(std::string const&, unsigned int, char = '=') const;
		void print_msg_bar(std::string const&, char = '=') const;

		// Generates a random solution. Returns the number of operations
		// performed. If exit_status is not set to Exhausted, then an
		// immediate call to this method will continue the previous run.
		template<const PathKind::E PK>
		[[gnu::hot]] GenResult generate(bool _continue = false);
		GenResult get_generate_result() { return generate_result; }

	// ========================
	 private:
		ord4_t (&path)(ord4_t progress);

		std::array<std::array<value_t, O2>, O2> val_try_order;
		std::array<Tile, O4> pathy_values;
		std::array<occmask_t, O2> rows_has;
		std::array<occmask_t, O2> cols_has;
		std::array<occmask_t, O2> blks_has;
		std::array<backtrack_t, O4> backtrack_counts;

		void clear(void);
		[[gnu::hot]] PathDirection set_next_valid(const ord4_t);
		struct GenResult generate_result;
	};

	const std::string GRID_SEP = "  ";
}

#endif