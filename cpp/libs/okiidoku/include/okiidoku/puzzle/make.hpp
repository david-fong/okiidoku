// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_PUZZLE_MAKE
#define HPP_OKIIDOKU_PUZZLE_MAKE

#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>
namespace okiidoku::mono { template <Order O> requires (is_order_compiled(O)) struct Grid; }
namespace okiidoku::visitor { struct Grid; }

namespace okiidoku::mono {


	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	/**
	a puzzle is a non-full grid. it is "proper" if the canonicalizations of all
	its fillings that follow the one rule are the same. i.e. it effectively only
	has one solution.
	\todo should this have a alternative function that checks whether there is
		_truly_ only one solution (no others- even if they are canonically equivalent)?
		how to implement this in a way that isn't wasteful? maybe an effort param to
		denote whether to check that solution set is canonically equivalent or short-
		circuit if size greater than one? also, maybe there should be a way for
		caller to get a copy of the solution? like an optional outparam?
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
	\post grid is a minimal puzzle whose solution canonicalizes to the same grid as
		the original input, and where every non-empty cell holds the same value as the
		same cell in the original input.
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