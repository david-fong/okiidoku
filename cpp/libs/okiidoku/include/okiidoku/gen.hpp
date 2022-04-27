#ifndef HPP_OKIIDOKU__GEN
#define HPP_OKIIDOKU__GEN

#include <okiidoku/shared_rng.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/order_templates.hpp>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT [[gnu::hot]] void generate(GridSpan<O> sink, SharedRng&);
}

namespace okiidoku::visitor {

	OKIIDOKU_EXPORT void generate(GridSpan sink, SharedRng&);
}
#endif