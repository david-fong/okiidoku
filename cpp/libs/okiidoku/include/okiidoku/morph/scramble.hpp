// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__MORPH__SCRAMBLE
#define HPP_OKIIDOKU__MORPH__SCRAMBLE

#include <okiidoku/morph/transform.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>


namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void scramble(Transformation<O>&, const rng_seed_t rng_seed) noexcept;
}


namespace okiidoku::visitor {

	OKIIDOKU_EXPORT void scramble(Transformation&, const rng_seed_t rng_seed) noexcept;
}
#endif