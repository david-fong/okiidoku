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
#include <ranges>
#include <numeric> // iota
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
		bool canonicalize = false;
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


	/** exists to defer copying to the caller. should be dropped before
	doing further generation. */
	struct ResultView final {
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
		using dead_ends_t = cell_dead_ends::t<O>;

	 public:
		static constexpr ord1i_t O1 = O;
		static constexpr ord2i_t O2 = O*O;
		static constexpr ord4i_t O4 = O*O*O*O;

		/** This exists in addition to the non-template Result class to
		losslessly preserve suitably sized number types. */
		class ResultView final {
			const Generator& g_;
		 public:
			explicit ResultView(const Generator& g): g_(g) {}
			[[nodiscard]]       auto  order()               const noexcept { return O; }
			[[nodiscard]] const auto& params()              const noexcept { return g_.params_; }
			[[nodiscard]]       auto  exit_status()         const noexcept { return g_.get_exit_status(); }
			[[nodiscard]] const auto& backtrack_origin()    const noexcept { return g_.backtrack_origin_; }
			[[nodiscard]] const auto& most_dead_ends_seen() const noexcept { return g_.most_dead_ends_seen_; }
			[[nodiscard]] const auto& op_count()            const noexcept { return g_.op_count_; }
			[[nodiscard]]       auto  get_grid()            const noexcept { return g_.get_grid(); }
			[[nodiscard]]       auto  get_dead_ends()       const noexcept { return g_.get_dead_ends(); }
			gen::ResultView to_generic() const { return g_.make_gen_result_(); } // TODO update ResultView to not copy out
		};

		// Generates a fresh sudoku solution.
		[[nodiscard]] ResultView operator()(Params);
		[[nodiscard]] ResultView continue_prev(void);

		[[nodiscard]] ExitStatus get_exit_status(void) const noexcept;

		auto get_grid() const noexcept {
			path::coord_converter_t<O> c2p = path::get_coord2prog_converter<O>(params_.path_kind); // TODO why does this segfault when using auto?
			return std::views::iota(ord4x_t{0}, O4)
			| std::views::transform([&](auto coord){
				const auto p = c2p(coord);
				const auto try_index = cells_[p].try_index;
				if (try_index == O2) { return O2; }
				return val_try_orders_[p/O2][try_index];
			});
		}
		auto get_dead_ends() const noexcept {
			path::coord_converter_t<O> c2p = path::get_coord2prog_converter<O>(params_.path_kind);
			return std::views::iota(ord4x_t{0}, O4)
			| std::views::transform([&](auto coord){ return dead_ends_[c2p(coord)]; });
		}

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

		[[gnu::pure, nodiscard]] gen::ResultView make_gen_result_(void) const;
	};


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		extern template class Generator<O_>;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
#endif