#ifndef HPP_OKIIDOKU__MONO__MORPH__CANON
#define HPP_OKIIDOKU__MONO__MORPH__CANON

#include <okiidoku/mono/morph/transform.hpp>
#include <okiidoku/mono/grid.hpp>
#include <okiidoku/traits.hpp>
#include <okiidoku/okiidoku_config.hpp>
#include <okiidoku_export.h>

namespace okiidoku::mono::morph {

	// contract: the span is a _complete_, valid grid.
	// If you want to canonicalize a puzzle, first solve it, then canonicalize
	// the solution, and then apply the returned transformation to the puzzle.
	template<Order O>
	requires (is_order_compiled(O))
	OKIIDOKU_EXPORT Transformation<O> canonicalize(grid_span_t<O>);
}
#endif