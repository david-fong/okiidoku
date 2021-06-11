#ifndef HPP_SOLVENT_LIB_GEN
#define HPP_SOLVENT_LIB_GEN

#include <solvent_lib/gen/path.hpp>
#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

// #include <iosfwd>
#include <random>
#include <array>
#include <string>
#include <optional>


namespace solvent::lib::gen {

	// must be manually seeded!
	extern std::mt19937 Rng;

	//
	struct Params {
		path::Kind path_kind;
		unsigned long max_backtracks = 0; // If zero, a default value will be used.
		template<Order O> Params clean(void) noexcept;
	};

	// Container for a very large number.
	// number of operations taken to generate a solution by grid-order.
	using opcount_t = unsigned long long;

	//
	enum class ExitStatus : unsigned {
		Exhausted, Abort, Ok,
	};

	//
	struct Direction {
		bool is_back: 1;
		bool is_skip: 1 = false; // only meaningful when is_back is true.
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
		using backtrack_t =
			// Make sure this can fit `DEFAULT_MAX_BACKTRACKS`.
			//typename std::conditional_t<(O < 4), std::uint_fast8_t,
			std::conditional_t<(O < 5), std::uint_fast16_t,
			std::conditional_t<(O < 6), std::uint_fast32_t,
			std::uint64_t
		>>;

		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;

		static constexpr backtrack_t DEFAULT_MAX_BACKTRACKS = [](){ const backtrack_t _[] = {
			0, 1, 3, 150, 700, 1'000'000, 1'000'000'000,
		}; return _[O]; }();

		//
		struct GenResult final {
			Params params; // Mainly used internally for continuation.
			ExitStatus status = ExitStatus::Abort;
			ord4_t progress = 0;
			opcount_t op_count = 0;
			backtrack_t most_backtracks_seen = 0;
			std::array<ord2_t, O4> grid = {};
		};

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

		// Pass std::nullopt to continue the previous run.
		[[gnu::hot]] GenResult generate(std::optional<Params>);
		[[gnu::pure]] GenResult const& get_gen_result() const { return gen_result_; }
		[[gnu::pure]] std::array<backtrack_t, O4> const& get_backtracks() const { return backtracks_; }

	 private:
		std::array<std::array<ord2_t, O2>, O2> val_try_orders_; // indexed by (progress/O2)
		std::array<Tile, O4> values_; // indexed by progress
		std::array<has_mask_t, O2> rows_has_;
		std::array<has_mask_t, O2> cols_has_;
		std::array<has_mask_t, O2> blks_has_;
		std::array<backtrack_t, O4> backtracks_; // indexed by progress

		// clear fields and scramble val_try_order
		void init(void);

		[[gnu::hot]] Direction set_next_valid(
			ord4_t progress,
			ord4_t dead_end_progress,
			ord4_t (& prog2coord)(ord4_t)
		) noexcept;

		GenResult gen_result_;
	};


	[[gnu::const]] std::string shaded_backtrack_stat(const long out_of, const long count);


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template Params Params::clean<O_>(void) noexcept; \
		extern template class Generator<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif