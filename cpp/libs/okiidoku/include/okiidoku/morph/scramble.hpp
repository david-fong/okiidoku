// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_MORPH_SCRAMBLE
#define HPP_OKIIDOKU_MORPH_SCRAMBLE
#include <okiidoku/detail/export.h>

#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>
namespace okiidoku::mono { template <Order O> requires (is_order_compiled(O)) struct Transformation; }
namespace okiidoku::visitor { struct Transformation; }


namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void scramble(Transformation<O>&, rng_seed_t rng_seed) noexcept;
}


namespace okiidoku::visitor {

	OKIIDOKU_EXPORT void scramble(Transformation&, rng_seed_t rng_seed) noexcept;
}
#endif