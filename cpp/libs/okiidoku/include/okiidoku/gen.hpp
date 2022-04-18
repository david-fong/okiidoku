#ifndef HPP_OKIIDOKU__GEN
#define HPP_OKIIDOKU__GEN

#include <okiidoku/shared_rng.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/traits.hpp>
#include <okiidoku/prelude.hpp>
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

		void operator()(SharedRng&);
		void write_to(GridSpan<O> sink) const;

	private:
		GridArr<O, val_t> cells_ {[]() consteval {
			GridArr<O, val_t> _;
			for (auto vto : _.rows()) { for (o2i_t i {0}; i < T::O2; ++i) { vto[i] = static_cast<val_t>(i); } }
			// ^ didn't want to import <algorithm> just for std::iota
			return _;
		}()};

		std::minstd_rand rng_; // other good LCG parameters https://arxiv.org/pdf/2001.05304v3.pdf
		// TODO.try using mt19937_64 isn't much slower... could it be okay to use?

		OKIIDOKU_NO_EXPORT [[gnu::hot]] void generate_();
	};
}
#endif