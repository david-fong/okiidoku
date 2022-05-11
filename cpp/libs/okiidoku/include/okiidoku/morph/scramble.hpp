#ifndef HPP_OKIIDOKU__MORPH__SCRAMBLE
#define HPP_OKIIDOKU__MORPH__SCRAMBLE

#include <okiidoku/shared_rng.hpp>
#include <okiidoku/morph/transform.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

namespace okiidoku::mono::morph {

	// post-condition: before and after states of the grid canonicalize to the same grid.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT Transformation<O> scramble(Grid<O>&, SharedRng&) noexcept;
}


namespace okiidoku::visitor::morph {

	// post-condition: before and after states of the grid canonicalize to the same grid.
	OKIIDOKU_EXPORT Transformation scramble(Grid&, SharedRng&) noexcept;
}
#endif