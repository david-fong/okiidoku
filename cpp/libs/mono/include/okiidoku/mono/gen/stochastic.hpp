#ifndef HPP_OKIIDOKU__MONO__GEN__STOCHASTIC
#define HPP_OKIIDOKU__MONO__GEN__STOCHASTIC

#include <okiidoku/mono/grid.hpp>
#include <okiidoku/traits.hpp>
#include <okiidoku/okiidoku_config.hpp>
#include <okiidoku_export.h>

#include <random>  // minstd_rand
#include <array>
#include <span>
#include <cassert>

namespace okiidoku::mono::gen::ss {

	// used for manual branch likelihood profiling
	// OKIIDOKU_EXPORT extern unsigned long long total;
	// OKIIDOKU_EXPORT extern unsigned long long true_;


	//
	template<Order O>
	requires (is_order_compiled(O))
	class OKIIDOKU_EXPORT Generator final {
	public:
		using T = traits<O>;
		using val_t = T::o2x_smol_t;
		using o1i_t = T::o1i_t;
		using o2x_t = T::o2x_t;
		using o2i_t = T::o2i_t;
		using o4x_t = T::o4x_t;
		using o4i_t = T::o4i_t;

		void operator()();
		void write_to(grid_span_t<O> sink) const;

	private:
		grid_arr2d_t<T::O1, val_t> cells_ {[]() consteval {
			grid_arr2d_t<T::O1, val_t> _;
			for (auto& vto : _) { for (o2i_t i {0}; i < T::O2; ++i) { vto[i] = static_cast<val_t>(i); } }
			// ^ didn't want to import <algorithm> just for std::iota
			return _;
		}()};

		std::minstd_rand rng_; // other good LCG parameters https://arxiv.org/pdf/2001.05304v3.pdf
		// TODO.try using mt19937_64 isn't much slower... could it be okay to use?

		OKIIDOKU_NO_EXPORT [[gnu::hot]] void generate_();
	};


	#define M_OKIIDOKU_TEMPL_TEMPL(O_) \
		extern template class Generator<O_>;
	M_OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OKIIDOKU_TEMPL_TEMPL
}
#endif