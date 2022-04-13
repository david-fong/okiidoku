#ifndef HPP_OKIIDOKU__VISITOR__MORPH__SCRAMBLE
#define HPP_OKIIDOKU__VISITOR__MORPH__SCRAMBLE

#include <okiidoku/visitor/morph/transform.hpp>
#include <okiidoku/visitor/grid.hpp>
#include <okiidoku/traits.hpp>
#include <okiidoku/compiled_orders.hpp>
#include <okiidoku_export.h>

namespace okiidoku::visitor::morph {

	// contract: span is a valid grid (though it may be incomplete)
	OKIIDOKU_EXPORT Transformation scramble(grid_span_t);
}
#endif