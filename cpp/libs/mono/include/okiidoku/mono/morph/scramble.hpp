#ifndef HPP_OKIIDOKU__MONO__MORPH__SCRAMBLE
#define HPP_OKIIDOKU__MONO__MORPH__SCRAMBLE

#include <okiidoku/mono/morph/transform.hpp>
#include <okiidoku/mono/grid.hpp>
#include <okiidoku/traits.hpp>
#include <okiidoku/okiidoku_config.hpp>
#include <okiidoku_export.h>

namespace okiidoku::mono::morph {

	// contract: span is a valid grid (though it may be incomplete)
	template<Order O>
	requires (is_order_compiled(O))
	OKIIDOKU_EXPORT Transformation<O> scramble(grid_span_t<O>);
}
#endif