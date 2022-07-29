// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__PUZZLE__MAKE
#define HPP_OKIIDOKU__PUZZLE__MAKE

#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

namespace okiidoku::mono {

	// contract: grid is filled or a proper puzzle.
	// post-condition: the grid is a minimal puzzle. solution unchanged.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT
	void make_minimal_puzzle(Grid<O>&, rng_seed_t rng_seed) noexcept;

	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	bool grid_is_proper_puzzle(const Grid<O>&) noexcept;

	// contract: grid is a proper puzzle.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	bool grid_is_minimal_puzzle(const Grid<O>&) noexcept;
}


namespace okiidoku::visitor {

	// contract: grid is filled or a proper puzzle.
	// post-condition: the grid is a minimal puzzle. solution unchanged.
	OKIIDOKU_EXPORT void make_minimal_puzzle(Grid&, rng_seed_t rng_seed) noexcept;

	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	bool grid_is_proper_puzzle(const Grid&) noexcept;

	// contract: grid is a proper puzzle.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	bool grid_is_minimal_puzzle(const Grid&) noexcept;
}
#endif