#ifndef HPP_OKIIDOKU__GEN
#define HPP_OKIIDOKU__GEN

#include <okiidoku/shared_rng.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <concepts>
#include <type_traits>

namespace okiidoku::mono {

	// post-condition: `sink` is filled and follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	[[gnu::hot]] OKIIDOKU_EXPORT void generate(Grid<O>& sink, SharedRng&) noexcept;
}

namespace okiidoku::visitor {

	// post-condition: `sink` is filled and follows the one rule.
	OKIIDOKU_EXPORT void generate(Grid& sink, SharedRng&) noexcept;
}
#endif