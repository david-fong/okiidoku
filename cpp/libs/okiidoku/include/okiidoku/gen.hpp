// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_GEN
#define HPP_OKIIDOKU_GEN

#include <okiidoku/grid.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

namespace okiidoku::mono {

	/**
	result is a function of the seed and the input state.
	\pre `grid` is filled and rows follow the one rule.
	\post `grid` is filled and follows the one rule. */
	template<Order O> requires(is_order_compiled(O))
	[[gnu::hot]] OKIIDOKU_EXPORT
	void shuffle(Grid<O>& grid, rng_seed_t rng_seed) noexcept;


	/**
	\return a random filled grid that follows the one rule.
	\note for large grids, this won't have good coverage of all possible random grids.
		the domain of the function (possible seeds) will limit the range.
	\internal perhaps we could do better by using a better PRNG... (PCG?) and passing
		the PRNG itself by reference. may be good for compiler optimization to make a
		local clone, then reassign it back to the arg before returning. or even just take
		by value, and leave advancing the PRNG to the caller. */
	template<Order O> requires(is_order_compiled(O)) [[nodiscard, gnu::const]]
	inline Grid<O> generate_shuffled(const rng_seed_t rng_seed) noexcept {
		using T = Ints<O>;
		Grid<O> grid OKIIDOKU_DEFER_INIT;
		#pragma clang loop unroll(disable)
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