#ifndef HPP_OKIIDOKU__VISITOR__MORPH__CANON
#define HPP_OKIIDOKU__VISITOR__MORPH__CANON

#include <okiidoku/visitor/grid.hpp>
#include <okiidoku/visitor/morph/transform.hpp>
#include <okiidoku/traits.hpp>
#include <okiidoku/compiled_orders.hpp>
#include <okiidoku_export.h>

namespace okiidoku::visitor::morph {

	// contract: the span is a _complete_, valid grid.
	// If you want to canonicalize a puzzle, first solve it, then canonicalize
	// the solution, and then apply the returned transformation to the puzzle.
	OKIIDOKU_EXPORT Transformation canonicalize(grid_span_t);
}
#endif