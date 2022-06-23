#ifndef HPP_OKIIDOKU__PUZZLE__UNAVOIDABLE_SET
#define HPP_OKIIDOKU__PUZZLE__UNAVOIDABLE_SET

#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	struct MinimalUnavoidableSets final {
		Grid<O> ids {}; // TODO dummy field. need to figure out actual representation. This doesn't work if a cell can be part of multiple minimal unavoidable sets.
	};

	// https://www.sudopedia.org/wiki/Deadly_Pattern
	// contract: grid is filled and follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard, gnu::pure]] OKIIDOKU_EXPORT
	MinimalUnavoidableSets<O> find_size_4_minimal_unavoidable_sets(const Grid<O>&) noexcept;
}


namespace okiidoku::visitor {
}
#endif