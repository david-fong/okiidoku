#ifndef HPP_OKIIDOKU__GEN
#define HPP_OKIIDOKU__GEN

#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

namespace okiidoku::mono {

	// post-condition: `sink` is filled and follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	[[gnu::hot]] OKIIDOKU_EXPORT void generate(Grid<O>& sink, rng_seed_t rng_seed) noexcept;
}

namespace okiidoku::visitor {

	// post-condition: `sink` is filled and follows the one rule.
	OKIIDOKU_EXPORT void generate(Grid& sink, rng_seed_t rng_seed) noexcept;
}
#endif