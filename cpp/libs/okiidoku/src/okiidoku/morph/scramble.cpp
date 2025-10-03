// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/morph/scramble.hpp>

#include <okiidoku/morph/transform.hpp> // Transformation
#include <okiidoku/ints.hpp>            // rng_seed_t, Ints, o1i_t
#include <okiidoku/order.hpp>

#include <algorithm> // ranges::shuffle
#include <random>    // minstd_rand
#include <array>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	void scramble(Transformation<O>& it, const rng_seed_t rng_seed) noexcept {
		// using T = Ints<O>;
		namespace stdr = std::ranges;
		{
			using rng_t = std::minstd_rand;
			rng_t rng {rng_seed};
			stdr::shuffle(it.sym_map, rng);
			stdr::shuffle(it.row_map, rng); for (auto& boxline_map : it.row_map) { stdr::shuffle(boxline_map, rng); }
			stdr::shuffle(it.col_map, rng); for (auto& boxline_map : it.col_map) { stdr::shuffle(boxline_map, rng); }
			it.post_transpose = static_cast<bool>((rng()-rng_t::min()) % 2u);
		}
	}


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template void scramble<(O_)>(Transformation<(O_)>&, const rng_seed_t) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}


namespace okiidoku::visitor {

	void scramble(Transformation& it, const rng_seed_t rng_seed) noexcept {
		switch (it.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return mono::scramble(it.unchecked_get_mono_exact<(O_)>(), rng_seed);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}
}