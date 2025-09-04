// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__PUZZLE__MAKE
#define HPP_OKIIDOKU__PUZZLE__MAKE

#include <okiidoku/grid.hpp>
#include <okiidoku/order.hpp>

namespace okiidoku::mono {


	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	/**
	a puzzle is a non-full grid. it is "proper" if the canonicalizations of all
	its fillings that follow the one rule are the same. i.e. it effectively only
	has one solution.
	\relates okiidoku::mono::Grid */
	bool grid_is_proper_puzzle(const Grid<O>&) noexcept;


	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	/**
	\returns `true` if clearing any cell of the puzzle would make it improper
		(see `okiidoku::mono::grid_is_proper_puzzle`).
	\pre `grid` is a proper puzzle.
	\relates okiidoku::mono::Grid */
	bool grid_is_minimal_puzzle(const Grid<O>& grid) noexcept;


	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT
	/**
	\pre grid is filled or a proper puzzle (see `okiidoku::mono::grid_is_proper_puzzle`).
	\post grid is a minimal puzzle whose solution canonicalizes to the same grid
		as the original input.
	\relates okiidoku::mono::Grid */
	void make_minimal_puzzle(Grid<O>&, rng_seed_t rng_seed) noexcept;
}


namespace okiidoku::visitor {

	/// see `okiidoku::mono::grid_is_proper_puzzle`
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	bool grid_is_proper_puzzle(const Grid&) noexcept;

	/// see `okiidoku::mono::grid_is_minimal_puzzle`
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	bool grid_is_minimal_puzzle(const Grid&) noexcept;

	/// see `okiidoku::mono::make_minimal_puzzle`
	OKIIDOKU_EXPORT void make_minimal_puzzle(Grid&, rng_seed_t rng_seed) noexcept;
}
#endif