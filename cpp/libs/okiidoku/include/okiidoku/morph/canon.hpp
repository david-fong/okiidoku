#ifndef HPP_OKIIDOKU__MORPH__CANON
#define HPP_OKIIDOKU__MORPH__CANON

#include <okiidoku/morph/transform.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

namespace okiidoku::mono {

	// contract: the span is a _complete_, valid grid.
	// If you want to canonicalize a puzzle, first solve it, then canonicalize
	// the solution, and then apply the returned transformation to the puzzle.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT Transformation<O> canonicalize(Grid<O>&) noexcept;
}


namespace okiidoku::visitor {

	// contract: the span is a _complete_, valid grid.
	// If you want to canonicalize a puzzle, first solve it, then canonicalize
	// the solution, and then apply the returned transformation to the puzzle.
	OKIIDOKU_EXPORT Transformation canonicalize(Grid&) noexcept;
}
#endif