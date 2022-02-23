#ifndef HPP_SOLVENT_LIB__GEN
#define HPP_SOLVENT_LIB__GEN

#include <solvent_lib/gen/path.hpp>
#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>
#include <solvent_config.hpp>

#include <iosfwd>
#include <vector>
#include <array>
#include <span>
#include <numeric>   // iota,
#include <cassert>

namespace solvent::lib::gen {

	// used for manual branch likelihood profiling
	// extern long long total;
	// extern long long true_;

	// must be manually seeded in the main function!
	// Used for shuffling Generator.
	// RNG is shared between threads, guarded by mutex.
	// Note: Using thread_local instead does not cause any noticeable perf change.
	void seed_rng(std::uint_fast32_t) noexcept;


	namespace cell_dead_ends {
		// TODO these values are tuned for genpath=rowmajor. make one for each genpath? :/
		constexpr unsigned long long LIMIT_DEFAULT[]{ 0, 0, 3,
			/*3*/30,
			/*4*/700,
			/*5*/100'000, // changing to anything between this and 100K doesn't seem to have any significant difference? I only tested with gen_ok 20 though.
			/*6*/10'000'000ull, // <- not tested AT ALL ...
			/*7*/10'000'000'000ull // <- not tested AT ALL ...
		};
		// allows up to and including
		constexpr unsigned long long LIMIT_I_MAX[]{ 0, 0, 3,
			/*3*/1'000,
			/*4*/10'000,
			/*5*/100'000'000ull, //  <- not tested AT ALL ...
			/*6*/10'000'000'000ull // <- not tested AT ALL ...
		};
		template<Order O>
		using t = uint_leastN_t<std::bit_width(LIMIT_I_MAX[O])>;
	};


	//
	struct Params {
		path::Kind path_kind = path::Kind::RowMajor;
		bool canonicalize = false; // TODO try moving up layout-wise and see if perf improves
		std::uint_fast64_t max_dead_ends = 0; // Defaulted if zero.

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
	// TODO would be cool to have arena allocator support for the vectors
	struct GenResult final {
	 public:
		using val_t = size<O_MAX>::ord2i_t;
		using dead_ends_t = float; // an experiment to save space when buffering batched results.
		using backtrack_origin_t = size<O_MAX>::ord4x_least_t;

		std::uint8_t O;
		Params params;
		ExitStatus status;
		backtrack_origin_t backtrack_origin;
		dead_ends_t most_dead_ends_seen;
		opcount_t op_count;
		std::vector<val_t> grid;
		std::vector<dead_ends_t> dead_ends;

		template<Order O_>
		std::span<const val_t, O_*O_*O_*O_> grid_const_span() const {
			assert(O_ == O);
			std::span s{grid};
			return s.template subspan<0, O_*O_*O_*O_>();
		}
		void print_text(std::ostream&) const;
		void print_pretty(std::ostream&) const;
	};

	//
	struct Direction final {
		bool is_back;
		bool is_back_skip; // only meaningful when is_back is true.
	};

	//
	template<Order O>
	class Generator final {
	 static_assert((O > 0) && (O <= O_MAX) && (O < 6)); // added restriction for sanity
	 private:
		using has_mask_t = size<O>::O2_mask_fast_t;
		using ord1i_t = size<O>::ord1i_t;
		using ord2i_t = size<O>::ord2i_t;
		using ord4i_t = size<O>::ord4i_t;
		using ord4x_t = size<O>::ord4x_t;

	 public:
		using dead_ends_t = cell_dead_ends::t<O>;

		static constexpr ord1i_t O1 = O;
		static constexpr ord2i_t O2 = O*O;
		static constexpr ord4i_t O4 = O*O*O*O;

		// Generates a fresh sudoku solution.
		[[nodiscard]] GenResult operator()(Params);
		[[nodiscard]] GenResult continue_prev(void);

		[[nodiscard]] const Params& get_params() const { return params_; }
		[[nodiscard]] ExitStatus get_exit_status(void) const noexcept;

	 private:
		struct Cell final {
			ord2i_t try_index; // Index into val_try_orders_. O2 if clear.
			void clear(void) noexcept { try_index = O2; }
			[[gnu::pure, nodiscard]] bool is_clear(void) const noexcept { return try_index == O2; }
		};

		// indexed by `progress_ // O2`
		std::array<std::array<typename size<O>::ord2x_t, O2>, O2> val_try_orders_ {[]() {
			std::array<std::array<typename size<O>::ord2x_t, O2>, O2> _;
			for (auto& vto : _) { std::iota(vto.begin(), vto.end(), 0); }
			return _;
		}()};

		std::array<Cell, O4> cells_; // indexed by progress_
		std::array<has_mask_t, O2> rows_has_;
		std::array<has_mask_t, O2> cols_has_;
		std::array<has_mask_t, O2> blks_has_;
		std::array<dead_ends_t, O4> dead_ends_; // indexed by progress_

		Params params_;
		ord4i_t progress_ = 0;
		uint_fastN_t<std::bit_width(O4)> backtrack_origin_ = 0;
		dead_ends_t most_dead_ends_seen_ = 0;
		opcount_t op_count_ = 0;

		// clear fields and scramble val_try_orders_
		void prepare_fresh_gen_(void);

		[[gnu::hot]] Direction set_next_valid_(typename path::coord_converter_t<O>, bool backtracked) noexcept;
		[[gnu::hot]] void generate_();

		[[gnu::pure, nodiscard]] GenResult make_gen_result_(void) const;
	};


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		extern template class Generator<O_>;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
#endif