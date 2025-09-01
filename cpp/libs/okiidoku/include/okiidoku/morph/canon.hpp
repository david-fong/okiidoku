// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__MORPH__CANON
#define HPP_OKIIDOKU__MORPH__CANON

#include <okiidoku/morph/transform.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

namespace okiidoku::mono {

	/**
	\pre the grid is filled and follows the one rule.
	to canonicalize a puzzle, first solve it, then canonicalize
	the solution, and then apply the returned transformation to the puzzle.
	\todo consider whether it's possible to implement support for puzzles without requiring caller to first solve.
	*/
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT Transformation<O> canonicalize(Grid<O>&) noexcept;
}


namespace okiidoku::visitor {

	/** \copydoc okiidoku::mono::canonicalize */
	OKIIDOKU_EXPORT Transformation canonicalize(Grid&) noexcept;
}
#endif