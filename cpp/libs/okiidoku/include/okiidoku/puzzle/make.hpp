#ifndef HPP_OKIIDOKU__PUZZLE__MAKE_HARD
#define HPP_OKIIDOKU__PUZZLE__MAKE_HARD

#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	struct DeadlyPatterns final {
		Grid<O> ids {}; // TODO dummy field. need to figure out actual representation.
	};

	// https://www.sudopedia.org/wiki/Deadly_Pattern
	// contract: grid is filled and follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT DeadlyPatterns<O> find_deadly_patterns(const Grid<O>&) noexcept;

	// contract: grid is filled or a proper puzzle.
	// post-condition: the grid is a minimal puzzle. solution unchanged.
	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT void make_minimal_puzzle(Grid<O>&, rng_seed_t rng_seed) noexcept;

	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT bool grid_is_proper_puzzle(const Grid<O>&) noexcept;

	// contract: grid is a proper puzzle.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT bool grid_is_minimal_puzzle(const Grid<O>&) noexcept;
}


namespace okiidoku::visitor {

	// contract: grid is filled or a proper puzzle.
	// post-condition: the grid is a minimal puzzle. solution unchanged.
	OKIIDOKU_EXPORT void make_minimal_puzzle(Grid&, rng_seed_t rng_seed) noexcept;

	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT bool grid_is_proper_puzzle(const Grid&) noexcept;

	// contract: grid is a proper puzzle.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT bool grid_is_minimal_puzzle(const Grid&) noexcept;
}
#endif