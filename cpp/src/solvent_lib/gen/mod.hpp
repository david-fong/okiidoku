#ifndef HPP_SOLVENT_LIB_GEN
#define HPP_SOLVENT_LIB_GEN

#include <solvent_lib/gen/path.hpp>
#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

#include <random>
#include <vector>
#include <array>
#include <string>


namespace solvent::lib::gen {

	// must be manually seeded!
	extern std::mt19937 Rng;

	//
	struct Params {
		path::Kind path_kind;
		unsigned long max_dead_ends = 0; // Defaulted if zero.
		Params clean(Order O) noexcept;
	};

	// Container for a very large number.
	// number of operations taken to generate a solution by grid-order.
	using opcount_t = unsigned long long;

	//
	enum class ExitStatus : unsigned char {
		Exhausted, Abort, Ok,
	};

	//
	struct GenResult final {
		Order O;
		ExitStatus status;
		unsigned long dead_end_progress;
		unsigned long long most_dead_ends_seen;
		opcount_t op_count;
		std::vector<unsigned char> grid = {};

		void print_serial(std::ostream&) const;
		void print_pretty(std::ostream&) const;
	};

	//
	struct Direction final {
		bool is_back: 1;
		bool is_skip: 1 = false; // only meaningful when is_back is true.
	};

	constexpr unsigned long long DEFAULT_MAX_DEAD_ENDS(const Order O) {
		const unsigned long long _[] = { 0, 0, 3, 100, 320, 100'000, 10'000'000 };
		return _[O];
	};

	//
	template<Order O>
	class Generator final : public Grid<O> {
	 private:
		using has_mask_t = typename size<O>::has_mask_t;
		using ord1_t = typename size<O>::ord1_t;
		using ord2_t = typename size<O>::ord2_t;
		using ord4_t = typename size<O>::ord4_t;

	 public:
		// Note that this should always be smaller than opcount_t.
		using dead_ends_t =
			// Make sure this can fit `DEFAULT_MAX_DEAD_ENDS`.
			// std::conditional_t<(O <= 3), std::uint_fast8_t,
			std::conditional_t<(O <= 4), std::uint_fast16_t,
			std::conditional_t<(O <= 5), std::uint_fast32_t,
			std::uint64_t
		>>;

		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;

		//
		struct Tile final {
			// Index into val_try_orders_. If set to O2, backtrack next.
			ord2_t next_try_index;
			ord2_t value;
			void clear(void) noexcept {
				next_try_index = 0;
				value = O2;
			}
			[[gnu::pure]] bool is_clear(void) const noexcept {
				return value == O2;
			}
		};

	 public:
		Generator(void);

		// Generates a fresh sudoku solution.
		GenResult operator()(Params);
		GenResult continue_prev();
		[[gnu::pure]] std::array<dead_ends_t, O4> const& get_dead_ends() const { return dead_ends_; }

	 private:
		std::array<std::array<ord2_t, O2>, O2> val_try_orders_; // indexed by (progress/O2)
		std::array<Tile, O4> values_; // indexed by progress
		std::array<has_mask_t, O2> rows_has_;
		std::array<has_mask_t, O2> cols_has_;
		std::array<has_mask_t, O2> blks_has_;
		std::array<dead_ends_t, O4> dead_ends_; // indexed by progress

		Params params_;
		ord4_t progress_ = 0;
		ord4_t dead_end_progress_ = 0;
		dead_ends_t most_dead_ends_seen_ = 0;
		opcount_t op_count_ = 0;
		ExitStatus prev_gen_status_ = ExitStatus::Abort;

		// clear fields and scramble val_try_orders_
		void prepare_fresh_gen(void);

		[[gnu::hot]] inline Direction set_next_valid(ord4_t (& prog2coord)(ord4_t)) noexcept;
		[[gnu::hot]] void generate(void);

		GenResult make_gen_result(void) const;
	};


	[[gnu::const]] std::string shaded_dead_end_stat(const long out_of, const long count);


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template class Generator<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif