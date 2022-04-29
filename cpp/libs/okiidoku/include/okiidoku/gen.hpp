#ifndef HPP_OKIIDOKU__GEN
#define HPP_OKIIDOKU__GEN

#include <okiidoku/shared_rng.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/order_templates.hpp>

#include <concepts>
#include <type_traits>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	OKIIDOKU_EXPORT [[gnu::hot]] void generate(GridSpan<O> sink, SharedRng&);

	// template-deducing auto-to-span-converting convenience wrapper
	// I don't know how it works, but I also don't know how else to get it to work :/
	void generate(auto& sink, SharedRng& rng) { return generate(GridSpan(sink), rng); }
}

namespace okiidoku::visitor {

	OKIIDOKU_EXPORT void generate(GridSpan sink, SharedRng&);
}
#endif