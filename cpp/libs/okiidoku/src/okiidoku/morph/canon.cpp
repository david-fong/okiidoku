#include <okiidoku/morph/canon.hpp>

#include <cassert>

namespace okiidoku::mono::detail {

	// contract: the grid is filled and follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	typename Transformation<O>::label_map_t canon_label(Grid<O>&) noexcept;

	// contract: the grid is filled and follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	Transformation<O> canon_place(Grid<O>&) noexcept;
}
namespace okiidoku::mono {

	// contract: `grid` is filled and follows the one rule.
	template<Order O> requires(is_order_compiled(O))
	Transformation<O> canonicalize(Grid<O>& grid) noexcept {
		assert(grid_is_filled(grid));
		assert(grid_follows_rule(grid));
		const auto label_map {detail::canon_label<O>(grid)};
		auto place_map {detail::canon_place<O>(grid)};
		// Transformation<O> place_map{}; // TODO.high delete when done canon_place
		place_map.label_map = label_map;
		return place_map;
	}

	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template Transformation<O_> canonicalize<O_>(Grid<O_>&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor {

	// contract: `vis_grid` is filled and follows the one rule.
	Transformation canonicalize(Grid& vis_grid) noexcept {
		switch (vis_grid.get_mono_order()) {
		#define OKIIDOKU_FOR_COMPILED_O(O_) \
		case O_: return static_cast<Transformation>(mono::canonicalize(vis_grid.unchecked_get_mono_exact<O_>()));
		OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef OKIIDOKU_FOR_COMPILED_O
		default: OKIIDOKU_CONTRACT_TRIVIAL_EVAL(false); // std::unreachable
		}
	}
}