#include <okiidoku/morph/canon.hpp>
#include <okiidoku/morph/transform.hpp>

#include <cassert>

namespace okiidoku::morph {

	// contract: the span is a _complete_, valid grid.
	template<Order O>
	requires (is_order_compiled(O))
	Transformation<O>::label_map_t canon_label(grid_span_t<O>);

	// contract: the span is a _complete_, valid grid.
	template<Order O>
	requires (is_order_compiled(O))
	Transformation<O> canon_place(grid_span_t<O>);

	template<Order O>
	requires (is_order_compiled(O))
	Transformation<O> canonicalize(const grid_span_t<O> og_grid) {
		assert(grid_is_filled<O>(og_grid));
		assert(grid_follows_rule<O>(og_grid));
		const auto label_map = canon_label<O>(og_grid);
		// auto place_map = canon_place<O>(og_grid);
		Transformation<O> place_map{};
		place_map.label_map = label_map;
		return place_map;
	}


	template<class T>
	requires std::is_integral_v<T>
	void canonicalize(Order order, std::span<T> grid) {
		assert(is_order_compiled(order));
		assert(grid.size() >= order*order*order*order);
		switch (order) {
		#define M_OKIIDOKU_TEMPL_TEMPL(O_) \
			case O_: { \
				constexpr unsigned O4 = O_*O_*O_*O_; \
				using val_t = traits<O_>::o2i_smol_t; \
				std::array<val_t,O4> grid_resize; \
				for (unsigned i {0}; i < O4; ++i) { grid_resize[i] = static_cast<val_t>(grid[i]); } \
				canonicalize<O_>(std::span<val_t, O4>(grid_resize)); \
				for (unsigned i {0}; i < O4; ++i) { grid[i] = static_cast<T>(grid_resize[i]); } \
				break; \
			}
		M_OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef M_OKIIDOKU_TEMPL_TEMPL
		}
	}

	#define M_OKIIDOKU_TEMPL_TEMPL(O_) \
		extern template typename Transformation<O_>::label_map_t canon_label<O_>(grid_span_t<O_>); \
		extern template Transformation<O_> canon_place<O_>(grid_span_t<O_>); \
		template Transformation<O_> canonicalize<O_>(grid_span_t<O_>);
	M_OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OKIIDOKU_TEMPL_TEMPL

	template void canonicalize<traits<O_MAX>::o2i_smol_t>(Order O, std::span<traits<O_MAX>::o2i_smol_t>);
}