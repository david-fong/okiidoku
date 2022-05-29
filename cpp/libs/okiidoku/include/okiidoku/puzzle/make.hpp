#ifndef HPP_OKIIDOKU__PUZZLE__MAKE_HARD
#define HPP_OKIIDOKU__PUZZLE__MAKE_HARD

#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

namespace okiidoku::mono {

	// contract: grid is filled or a proper puzzle.
	// post-condition: the grid is a minimal puzzle. solution unchanged.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void make_minimal_puzzle(Grid<O>&, rng_seed_t rng_seed) noexcept;

	// Not decided on whether to put is_minimal and is_proper functions
	// here. is_proper can be defined in terms of solving, and is_minimal
	// can be defined in terms of is_proper. A minimal-puzzle generator
	// could make good use of those.

	// contract: grid is filled or a proper puzzle.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT bool is_minimal_puzzle(const Grid<O>&) noexcept;
}


namespace okiidoku::visitor {

	// contract: grid is filled or a proper puzzle.
	// post-condition: the grid is a minimal puzzle. solution unchanged.
	OKIIDOKU_EXPORT void make_minimal_puzzle(Grid&, rng_seed_t rng_seed) noexcept;

	// contract: grid is filled or a proper puzzle.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT bool is_minimal_puzzle(const Grid&) noexcept;
}
#endif