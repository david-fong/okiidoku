#include <ookiidoku/morph/canon.hpp>
#include <ookiidoku/morph/transform.hpp>

#include <cassert>

namespace ookiidoku::morph {

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
	Transformation<O> canonicalize(const grid_span_t<O> orig_grid) {
		assert(is_sudoku_filled<O>(orig_grid));
		assert(is_sudoku_valid<O>(orig_grid));
		const auto label_map = canon_label<O>(orig_grid);
		auto place_map = canon_place<O>(orig_grid);
		place_map.label_map = label_map;
		return place_map;
	}


	template<class T>
	requires std::is_integral_v<T>
	void canonicalize(Order order, std::span<T> grid) {
		assert(is_order_compiled(order));
		assert(grid.size() >= order*order*order*order);
		switch (order) {
		#define M_OOKIIDOKU_TEMPL_TEMPL(O_) \
			case O_: { \
				constexpr unsigned O4 = O_*O_*O_*O_; \
				using val_t = size<O_>::ord2i_least_t; \
				std::array<val_t,O4> grid_resize; \
				for (unsigned i {0}; i < O4; ++i) { grid_resize[i] = static_cast<val_t>(grid[i]); } \
				canonicalize<O_>(std::span<val_t, O4>(grid_resize)); \
				for (unsigned i {0}; i < O4; ++i) { grid[i] = static_cast<T>(grid_resize[i]); } \
				break; \
			}
		M_OOKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef M_OOKIIDOKU_TEMPL_TEMPL
		}
	}

	#define M_OOKIIDOKU_TEMPL_TEMPL(O_) \
		extern template typename Transformation<O_>::label_map_t canon_label<O_>(grid_span_t<O_>); \
		extern template Transformation<O_> canon_place<O_>(grid_span_t<O_>); \
		template Transformation<O_> canonicalize<O_>(grid_span_t<O_>);
	M_OOKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OOKIIDOKU_TEMPL_TEMPL

	template void canonicalize<size<O_MAX>::ord2i_least_t>(Order O, std::span<size<O_MAX>::ord2i_least_t>);
}