#ifndef HPP_OOKIIDOKU__GEN__STOCHASTIC
#define HPP_OOKIIDOKU__GEN__STOCHASTIC

#include <ookiidoku/grid.hpp>
#include <ookiidoku/traits.hpp>
#include <ookiidoku/ookiidoku_config.hpp>
#include <ookiidoku_export.h>

#include <random>  // minstd_rand
#include <array>
#include <span>
#include <memory>  // unique_ptr
#include <cassert>

namespace ookiidoku::gen::ss {

	// used for manual branch likelihood profiling
	// OOKIIDOKU_EXPORT extern unsigned long long total;
	// OOKIIDOKU_EXPORT extern unsigned long long true_;


	class OOKIIDOKU_EXPORT Generator {
	public:
		using val_t = traits<O_MAX>::o2x_smol_t;
		using coord_t = traits<O_MAX>::o4x_t;

		// contract: GeneratorO<O> is compiled
		static std::unique_ptr<Generator> create(Order O);

		virtual void operator()() = 0;

		[[nodiscard]] virtual Order get_order() const noexcept = 0;

		[[nodiscard]] virtual val_t get_val_at(coord_t) const noexcept = 0;

		// contract: `sink.size() >= O4`
		// this cannot statically check that T is wide enough. uses static_cast<T>.
		// ie. it is your job to make sure T does not lose precision.
		template<class T>
		requires std::is_integral_v<T> && (!std::is_const_v<T>)
		void write_to(std::span<T> sink) const {
			const unsigned O4 = get_order()*get_order()*get_order()*get_order();
			assert(sink.size() >= O4);
			for (unsigned i {0}; i < O4; ++i) { sink[i] = static_cast<T>(get_val_at(i)); }
		}
	};


	//
	template<Order O>
	requires (is_order_compiled(O))
	class OOKIIDOKU_EXPORT GeneratorO final : public Generator {
	public:
		using T = traits<O>;
		using val_t = T::o2x_smol_t;

		void operator()() override;

		[[nodiscard]] constexpr Order get_order() const noexcept { return O; }
		[[nodiscard, gnu::hot]] Generator::val_t get_val_at(Generator::coord_t coord) const noexcept { return static_cast<Generator::val_t>(get_val_at_(static_cast<T::o4x_t>(coord))); }

		[[nodiscard, gnu::hot]] val_t get_val_at_(T::o4x_t coord) const noexcept;

		template<class V>
		requires std::is_integral_v<V> && (!std::is_const_v<V>) && (sizeof(V) >= sizeof(val_t))
		void write_to_(std::span<V, T::O4> sink) const {
			assert(sink.size() >= T::O4);
			for (typename T::o4i_t i {0}; i < T::O4; ++i) { sink[i] = get_val_at_(static_cast<T::o4x_t>(i)); }
		}

	private:
		grid_arr2d_t<T::O1, val_t> cells_ {[]() consteval {
			grid_arr2d_t<T::O1, val_t> _;
			for (auto& vto : _) { for (typename T::o2i_t i {0}; i < T::O2; ++i) { vto[i] = static_cast<val_t>(i); } }
			// ^ didn't want to import <algorithm> just for std::iota
			return _;
		}()};

		std::minstd_rand rng_; // other good LCG parameters https://arxiv.org/pdf/2001.05304v3.pdf
		// TODO.try using mt19937_64 isn't much slower... could it be okay to use?

		OOKIIDOKU_NO_EXPORT [[gnu::hot]] void generate_();
	};


	#define M_OOKIIDOKU_TEMPL_TEMPL(O_) \
		extern template class GeneratorO<O_>;
	M_OOKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OOKIIDOKU_TEMPL_TEMPL
}
#endif