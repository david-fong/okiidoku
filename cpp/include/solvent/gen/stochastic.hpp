#ifndef HPP_SOLVENT__GEN__STOCHASTIC
#define HPP_SOLVENT__GEN__STOCHASTIC

#include "solvent/grid.hpp"
#include "solvent/size.hpp"
#include "solvent_config.hpp"
#include "solvent_export.h"

#include <random>  // minstd_rand
#include <array>
#include <span>
#include <memory>  // unique_ptr
#include <cassert>

namespace solvent::gen::ss {

	namespace opcount {
		// Container for a very large number.
		// number of operations taken to generate a solution by grid-order.
		using t = unsigned long long;

		constexpr unsigned long long limit_default[]{ 0, 0, 3,
			/*3*/30,
			/*4*/700,
			/*5*/100'000, // changing to anything between this and 100K doesn't seem to have any significant difference? I only tested with gen_ok 20 though.
			/*6*/10'000'000ull, // <- not tested AT ALL ...
			/*7*/10'000'000'000ull // <- not tested AT ALL ...
		};
		// allows up to and including
		constexpr unsigned long long limit_i_max[]{ 0, 0, 3,
			/*3*/1'000,
			/*4*/10'000,
			/*5*/100'000'000ull, //  <- not tested AT ALL ...
			/*6*/10'000'000'000ull // <- not tested AT ALL ...
		};

	}

	struct SOLVENT_EXPORT Params {
		opcount::t max_ops {0}; // Defaulted if zero.
		Params clean(Order O) noexcept; // Cleans self and returns a copy of self.
	};

	enum class SOLVENT_EXPORT ExitStatus : std::uint8_t {
		Ok, Abort, // Exhausted,
	};


	class SOLVENT_EXPORT Generator {
	public:
		using val_t = size<O_MAX>::ord2i_t;
		using coord_t = size<O_MAX>::ord4x_t;

		// contract: GeneratorO<O> is compiled
		static std::unique_ptr<Generator> create(Order O);

		virtual void operator()(Params) = 0;
		virtual void continue_prev() = 0;

		[[nodiscard]] virtual Order get_order() const noexcept = 0;
		[[nodiscard]] constexpr Order get_order2() const noexcept { return get_order()*get_order(); }
		[[nodiscard]] constexpr Order get_order4() const noexcept { return get_order2()*get_order2(); }
		[[nodiscard]] virtual const Params& get_params() const noexcept = 0;
		[[nodiscard]] virtual ExitStatus status() const noexcept = 0;
		[[nodiscard]] virtual opcount::t get_op_count() const noexcept = 0;

		// these are optimized for reading out specific entries _once_- not for
		// repeated calls with the same coord. For that, use write_to or to_vec.
		[[nodiscard]] virtual val_t extract_val_at(coord_t) const noexcept = 0;

		// this cannot statically check that T is wide enough. uses static_cast<T>.
		// ie. it is your job to make sure T does not lose precision.
		template<class T>
		requires std::is_integral_v<T> && (!std::is_const_v<T>)
		void write_to(std::span<T> sink) const {
			const unsigned O4 = get_order4();
			assert(sink.size() >= O4);
			for (unsigned i {0}; i < O4; ++i) { sink[i] = static_cast<T>(extract_val_at(i)); }
		}
	};


	//
	template<Order O>
	requires ((O > 0) && (O <= O_MAX) && (O < 6)) // added <6 restriction for sanity
	class SOLVENT_EXPORT GeneratorO final : public Generator {
	public:
		using ord1i_t = size<O>::ord1i_t;
		using ord2x_t = size<O>::ord2x_t;
		using ord2i_t = size<O>::ord2i_t;
		using ord4x_t = size<O>::ord4x_t;
		using ord4i_t = size<O>::ord4i_t;

		SOLVENT_NO_EXPORT static constexpr ord1i_t O1 = O;
		SOLVENT_NO_EXPORT static constexpr ord2i_t O2 = O*O;
		SOLVENT_NO_EXPORT static constexpr ord4i_t O4 = O*O*O*O;

		using has_count_t = std::array<size<O>::ord2i_t, O2>; // perf seemed similar and slightly better compared to fast_t

		void operator()(Params) override;
		void continue_prev() override;

		[[nodiscard]] constexpr Order get_order() const noexcept { return O; }
		[[nodiscard]] constexpr const Params& get_params() const noexcept { return params_; }
		[[nodiscard]] constexpr ExitStatus status() const noexcept {
			if (count_total_has_nots_ == 0) {
				return ExitStatus::Ok;
			} else {
				return ExitStatus::Abort;
			}
		}
		[[nodiscard]] constexpr opcount::t get_op_count() const noexcept { return op_count_; }
		[[nodiscard, gnu::hot]] Generator::val_t extract_val_at(Generator::coord_t coord) const noexcept { return static_cast<Generator::val_t>(extract_val_at_(static_cast<ord4x_t>(coord))); }

		[[nodiscard, gnu::hot]] ord2i_t extract_val_at_(ord4x_t coord) const noexcept;

		template<class T>
		requires std::is_integral_v<T> && (!std::is_const_v<T>) && (sizeof(T) >= sizeof(ord2i_t))
		void write_to_(std::span<T, O4> sink) const {
			assert(sink.size() >= O4);
			for (ord4i_t i {0}; i < O4; ++i) { sink[i] = extract_val_at_(i); }
		}

	private:
		Params params_;
		opcount::t op_count_ {0};
		uint_fastN_t<> count_total_has_nots_; // sum of counting zeros in blks_has_ and cols_has_.
		std::minstd_rand rng_;

		std::array<std::array<ord2x_t, O2>, O2> cells_ {[]{
			std::array<std::array<ord2x_t, O2>, O2> _;
			for (auto& vto : _) { for (ord2i_t i {0}; i < O2; ++i) { vto[i] = i; } }
			return _;
		}()};

		std::array<has_count_t, O2> blks_has_; // max entry value: O1
		std::array<has_count_t, O2> cols_has_; // max entry value: 

		SOLVENT_NO_EXPORT [[gnu::hot]] void generate_();
		SOLVENT_NO_EXPORT [[gnu::hot]] void try_swap_() noexcept;
	};


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		extern template class GeneratorO<O_>;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
#endif