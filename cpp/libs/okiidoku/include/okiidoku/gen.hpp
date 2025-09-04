// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__GEN
#define HPP_OKIIDOKU__GEN

#include <okiidoku/ints.hpp> // for rng_seed_t
#include <okiidoku/order.hpp>
namespace okiidoku::mono { template <Order O> requires (is_order_compiled(O)) struct Grid; }
namespace okiidoku::visitor { struct Grid; }

namespace okiidoku::mono {

	// result is a function of the seed and the input state.
	// contract: `sink` is filled and rows follow the one rule.
	// post-condition: `sink` is filled and follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	[[gnu::hot]] OKIIDOKU_EXPORT void generate_shuffled(Grid<O>& sink, rng_seed_t rng_seed) noexcept;
}

namespace okiidoku::visitor {

	// result is a function of the seed and the input state.
	// contract: `sink` is filled and rows follow the one rule.
	// post-condition: `sink` is filled and follows the one rule.
	OKIIDOKU_EXPORT void generate_shuffled(Grid& sink, rng_seed_t rng_seed) noexcept;
}
#endif