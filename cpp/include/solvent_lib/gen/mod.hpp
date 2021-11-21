#ifndef HPP_SOLVENT_LIB__GEN
#define HPP_SOLVENT_LIB__GEN

#include <solvent_lib/gen/path.hpp>
#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

#include <iosfwd>
#include <vector>
#include <array>
#include <numeric>   // iota,

namespace solvent::lib::gen {

	// must be manually seeded in the main function!
	// Used for shuffling Generator.
	// Shared between threads, guarded by mutex.
	// Using thread_local instead does not cause any noticeable perf change.
	extern void seed_rng(std::uint_fast32_t) noexcept;

	//
	struct Params {
		path::Kind path_kind = path::Kind::RowMajor;
		unsigned long max_dead_ends = 0; // Defaulted if zero.
		bool canonicalize = false;

		// Cleans self and returns a copy of self.
		Params clean(Order O) noexcept;
	};

	// Container for a very large number.
	// number of operations taken to generate a solution by grid-order.
	using opcount_t = unsigned long long;

	//
	enum class ExitStatus : unsigned char {
		Ok, Abort, Exhausted,
	};

	//
	struct GenResult final {
		Order O;
		Params params;
		ExitStatus status;
		unsigned long frontier_progress;
		unsigned long long most_dead_ends_seen;
		opcount_t op_count;
		std::vector<std::uint_fast8_t> grid; // NOTE: assumes O1 < 16
		std::vector<std::uint_fast64_t> dead_ends;

		void print_serial(std::ostream&) const;
		void print_pretty(std::ostream&) const;
	};

	//
	struct Direction final {
		bool is_back;
		bool is_skip; // only meaningful when is_back is true.
	};

	constexpr unsigned long long DEFAULT_MAX_DEAD_ENDS(const Order O) {
		unsigned long long _[] = { 0, 0, 3, 100, 320, 100'000, 10'000'000 };
		return _[O];
	};

	//
	template<Order O>
	class Generator final {
	 static_assert(O > 0 && O < MAX_REASONABLE_ORDER);
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
			std::uint_fast64_t
		>>;

		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;

		// Generates a fresh sudoku solution.
		GenResult operator()(Params);
		GenResult continue_prev(void);

		const Params& get_params() const { return params_; }
		[[gnu::pure]] const std::array<dead_ends_t, O4>& get_dead_ends() const { return dead_ends_; }

	 private:
		//
		struct Cell final {
			// Index into val_try_orders_. O2 if clear.
			ord2_t try_index;
			void clear(void) noexcept {
				try_index = O2;
			}
			[[gnu::pure]] bool is_clear(void) const noexcept {
				return try_index == O2;
			}
		};

		// indexed by `progress // O2`
		// Note: a possibly more performant option would be `progress_ // O2`,
		// which the current simple setup doesn't accommodate for `operator[]`.
		std::array<std::array<ord2_t, O2>, O2> val_try_orders_ = []() {
			std::array<std::array<ord2_t, O2>, O2> val_try_orders;
			for (auto& vto : val_try_orders) {
				std::iota(vto.begin(), vto.end(), 0);
			}
			return val_try_orders;
		}();

		std::array<Cell, O4> cells_; // indexed by progress
		std::array<has_mask_t, O2> rows_has_;
		std::array<has_mask_t, O2> cols_has_;
		std::array<has_mask_t, O2> blks_has_;
		std::array<dead_ends_t, O4> dead_ends_; // indexed by progress

		Params params_;
		ord4_t progress_ = 0;
		ord4_t frontier_progress_ = 0;
		dead_ends_t most_dead_ends_seen_ = 0;
		opcount_t op_count_ = 0;
		ExitStatus prev_gen_status_ = ExitStatus::Ok;

		// clear fields and scramble val_try_orders_
		void prepare_fresh_gen_(void);

		[[gnu::hot]] Direction set_next_valid_(typename path::coord_converter_t<O>, bool backtracked) noexcept;
		[[gnu::hot]] void generate_();

		[[gnu::pure, nodiscard]] GenResult make_gen_result_(void) const;
	};


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template class Generator<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif