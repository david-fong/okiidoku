#include <okiidoku/morph/canon.hpp>

#include <cassert>

namespace okiidoku::mono::morph::detail {
	// contract: the span is a _complete_, valid grid.
	template<Order O> requires(is_order_compiled(O))
	typename Transformation<O>::label_map_t canon_label(Grid<O>&);

	// contract: the span is a _complete_, valid grid.
	template<Order O> requires(is_order_compiled(O))
	Transformation<O> canon_place(Grid<O>&);
}
namespace okiidoku::mono::morph {

	template<Order O> requires(is_order_compiled(O))
	Transformation<O> canonicalize(Grid<O>& og_grid) {
		assert(grid_is_filled<O>(og_grid));
		assert(grid_follows_rule<O>(og_grid));
		const auto label_map {detail::canon_label<O>(og_grid)};
		auto place_map {detail::canon_place<O>(og_grid)};
		// Transformation<O> place_map{}; // TODO.high delete when done canon_place
		place_map.label_map = label_map;
		return place_map;
	}

	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template Transformation<O_> canonicalize<O_>(Grid<O_>&);
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor::morph {

	Transformation canonicalize(Grid& vis_grid) {
		return std::visit([&](auto& mono_grid) {
			return static_cast<Transformation>(mono::morph::canonicalize(mono_grid));
		}, vis_grid.get_mono_variant());
	}
}