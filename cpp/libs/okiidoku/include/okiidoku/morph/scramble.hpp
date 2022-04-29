#ifndef HPP_OKIIDOKU__MORPH__SCRAMBLE
#define HPP_OKIIDOKU__MORPH__SCRAMBLE

#include <okiidoku/shared_rng.hpp>
#include <okiidoku/morph/transform.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/order_templates.hpp>

namespace okiidoku::mono::morph {

	// contract: span is a valid grid (though it may be incomplete)
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT Transformation<O> scramble(GridSpan<O>, SharedRng&);

	// template-deducing auto-to-span-converting convenience wrapper
	auto scramble(auto& grid, SharedRng& rng) { return scramble(GridSpan(grid), rng); }
}


namespace okiidoku::visitor::morph {

	// contract: span is a valid grid (though it may be incomplete)
	OKIIDOKU_EXPORT Transformation scramble(GridSpan, SharedRng&);
}
#endif