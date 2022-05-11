#ifndef HPP_OKIIDOKU__PUZZLE__MAKE_HARD
#define HPP_OKIIDOKU__PUZZLE__MAKE_HARD

// #include <okiidoku/shared_rng.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

namespace okiidoku::mono::puzzle {

	// contract: grid is filled or a proper puzzle.
	// post-condition: the grid is a minimal puzzle. solution unchanged.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void make_minimal(Grid<O>&) noexcept;

	// Not decided on whether to put is_minimal and is_proper functions
	// here. is_proper can be defined in terms of solving, and is_minimal
	// can be defined in terms of is_proper. A minimal-puzzle generator
	// could make good use of those.

	// contract: grid is filled or a proper puzzle.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard]] OKIIDOKU_EXPORT bool is_minimal(const Grid<O>&) noexcept;
}


namespace okiidoku::visitor::puzzle {

	// contract: grid is filled or a proper puzzle.
	// post-condition: the grid is a minimal puzzle. solution unchanged.
	OKIIDOKU_EXPORT void make_minimal(Grid&) noexcept;

	// contract: grid is filled or a proper puzzle.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard]] OKIIDOKU_EXPORT bool is_minimal(const Grid<O>&) noexcept;
}
#endif