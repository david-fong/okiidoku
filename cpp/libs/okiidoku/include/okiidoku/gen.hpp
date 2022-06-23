#ifndef HPP_OKIIDOKU__GEN
#define HPP_OKIIDOKU__GEN

#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

namespace okiidoku::mono {

	// wipes the grid before generating. result is a function of the seed only.
	// post-condition: `sink` is filled and follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	[[gnu::hot]] OKIIDOKU_EXPORT void generate(Grid<O>& sink, rng_seed_t rng_seed) noexcept;

	// result is a function of the seed and the input state.
	// contract: `sink` is filled and follows the one rule.
	// post-condition: `sink` is filled and follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	[[gnu::hot]] OKIIDOKU_EXPORT void generate_shuffled(Grid<O>& sink, rng_seed_t rng_seed) noexcept;
}

namespace okiidoku::visitor {

	// wipes the grid before generating. result is a function of the seed only.
	// post-condition: `sink` is filled and rows follow the rule.
	OKIIDOKU_EXPORT void generate(Grid& sink, rng_seed_t rng_seed) noexcept;

	// result is a function of the seed and the input state.
	// contract: `sink` is filled and follows the one rule.
	// post-condition: `sink` is filled and follows the one rule.
	OKIIDOKU_EXPORT void generate_shuffled(Grid& sink, rng_seed_t rng_seed) noexcept;
}
#endif