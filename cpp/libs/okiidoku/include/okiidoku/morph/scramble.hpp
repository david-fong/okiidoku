// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__MORPH__SCRAMBLE
#define HPP_OKIIDOKU__MORPH__SCRAMBLE

#include <okiidoku/morph/transform.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

namespace okiidoku::mono {

	// post-condition: before and after states of the grid canonicalize to the same grid.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT Transformation<O> scramble(Grid<O>&, rng_seed_t rng_seed) noexcept;
}


namespace okiidoku::visitor {

	// post-condition: before and after states of the grid canonicalize to the same grid.
	OKIIDOKU_EXPORT Transformation scramble(Grid&, rng_seed_t rng_seed) noexcept;
}
#endif