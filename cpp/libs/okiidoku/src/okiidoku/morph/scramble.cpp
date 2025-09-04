// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/morph/scramble.hpp>

#include <okiidoku/morph/transform.hpp> // Transformation
#include <okiidoku/grid.hpp>            // grid_follows_rule, Grid
#include <okiidoku/ints.hpp>            // rng_seed_t, Ints, o1i_t
#include <okiidoku/order.hpp>

#include <algorithm> // ranges::shuffle
#include <random>    // minstd_rand
#include <array>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	Transformation<O> scramble(Grid<O>& grid, const rng_seed_t rng_seed) noexcept {
		using T = Ints<O>;
		namespace stdr = std::ranges;
		Transformation<O> t {};
		{
			using rng_t = std::minstd_rand;
			rng_t rng {rng_seed}; // TODO try std::ranges::shuffle on my compilers
			stdr::shuffle(t.sym_map, rng);
			stdr::shuffle(t.row_map, rng);
			stdr::shuffle(t.col_map, rng);
			for (int_ts::o1i_t<O> chute {0}; chute < T::O1; ++chute) {
				stdr::shuffle(t.row_map[chute], rng);
				stdr::shuffle(t.col_map[chute], rng);
			}
			// t.post_transpose = static_cast<bool>(rng() % 2); // TODO add this back when canonicalize can handle it
		}
		t.apply_in_place(grid);
		OKIIDOKU_CONTRACT_ASSERT(grid_follows_rule<O>(grid));
		return t;
	}


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template Transformation<O_> scramble<O_>(Grid<O_>&, rng_seed_t) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}


namespace okiidoku::visitor {

	Transformation scramble(Grid& vis_grid, const rng_seed_t rng_seed) noexcept {
		switch (vis_grid.get_mono_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return static_cast<Transformation>(mono::scramble(vis_grid.unchecked_get_mono_exact<O_>(), rng_seed));
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}
}