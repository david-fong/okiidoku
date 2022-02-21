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

	// used for manual branch likelihood profiling
	// extern long long total;
	// extern long long true_;

	// must be manually seeded in the main function!
	// Used for shuffling Generator.
	// RNG is shared between threads, guarded by mutex.
	// Note: Using thread_local instead does not cause any noticeable perf change.
	void seed_rng(std::uint_fast32_t) noexcept;

	//
	struct Params {
		path::Kind path_kind = path::Kind::RowMajor;
		std::uint_fast64_t max_dead_ends = 0; // Defaulted if zero.
		bool canonicalize = false;

		// Cleans self and returns a copy of self.
		Params clean(Order O) noexcept;
	};

	// Container for a very large number.
	// number of operations taken to generate a solution by grid-order.
	using opcount_t = unsigned long long;

	//
	enum class ExitStatus : std::uint8_t {
		Ok, Abort, Exhausted,
	};

	//
	struct GenResult final {
		Order O;
		Params params;
		ExitStatus status;
		unsigned long backtrack_origin;
		std::uint_fast64_t most_dead_ends_seen;
		opcount_t op_count;
		std::vector<std::uint_fast8_t> grid; // NOTE: assumes O1 < 16
		std::vector<std::uint_fast64_t> dead_ends;

		void print_serial(std::ostream&) const;
		void print_pretty(std::ostream&) const;
	};

	//
	struct Direction final {
		bool is_back;
		bool is_back_skip; // only meaningful when is_back is true.
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
		using has_mask_t = typename size<O>::O2_mask_fast_t;
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
		[[nodiscard]] GenResult operator()(Params);
		[[nodiscard]] GenResult continue_prev(void);

		[[nodiscard]] const Params& get_params() const { return params_; }
		[[nodiscard]] ExitStatus get_exit_status(void) const noexcept;

	 private:
		struct Cell final {
			ord2_t try_index; // Index into val_try_orders_. O2 if clear.
			void clear(void) noexcept { try_index = O2; }
			[[gnu::pure, nodiscard]] bool is_clear(void) const noexcept { return try_index == O2; }
		};

		// indexed by `progress_ // O2`
		std::array<std::array<ord2_t, O2>, O2> val_try_orders_ {[]() {
			std::array<std::array<ord2_t, O2>, O2> _;
			for (auto& vto : _) { std::iota(vto.begin(), vto.end(), 0); }
			return _;
		}()};

		std::array<Cell, O4> cells_; // indexed by progress_
		std::array<has_mask_t, O2> rows_has_;
		std::array<has_mask_t, O2> cols_has_;
		std::array<has_mask_t, O2> blks_has_;
		std::array<dead_ends_t, O4> dead_ends_; // indexed by progress_

		Params params_;
		ord4_t progress_ = 0;
		ord4_t backtrack_origin_ = 0;
		dead_ends_t most_dead_ends_seen_ = 0;
		opcount_t op_count_ = 0;

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