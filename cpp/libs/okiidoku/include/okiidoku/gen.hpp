// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_GEN
#define HPP_OKIIDOKU_GEN

#include <okiidoku/ints.hpp> // for rng_seed_t
#include <okiidoku/order.hpp>
namespace okiidoku::mono { template <Order O> requires (is_order_compiled(O)) struct Grid; }
namespace okiidoku::visitor { struct Grid; }

namespace okiidoku::mono {

	/**
	result is a function of the seed and the input state.
	\pre `grid` is filled and rows follow the one rule.
	\post `grid` is filled and follows the one rule. */
	template<Order O> requires(is_order_compiled(O))
	[[gnu::hot]] OKIIDOKU_EXPORT
	void shuffle(Grid<O>& grid, rng_seed_t rng_seed) noexcept;

	/** \return a random filled grid that follows the one rule. */
	template<Order O> requires(is_order_compiled(O)) [[nodiscard]]
	inline Grid<O> generate_shuffled(const rng_seed_t rng_seed) noexcept {
		using T = Ints<O>;
		Grid<O> grid OKIIDOKU_DEFER_INIT;
		for (const auto row : T::O2) {
		for (const auto col : T::O2) {
			grid[row, col] = col;
		}}
		shuffle(grid, rng_seed);
		return grid;
	}
}

namespace okiidoku::visitor {

	OKIIDOKU_EXPORT void shuffle(Grid& sink, rng_seed_t rng_seed) noexcept;
	OKIIDOKU_EXPORT Grid generate_shuffled(Order O, rng_seed_t rng_seed) noexcept;
}
#endif