#include "solvent/morph/canon.hpp"

// #include <cassert>

namespace solvent::morph {

	template<Order O>
	void canon_label(grid_span_t<O>);

	template<Order O>
	void canon_place(grid_span_t<O>);

	template<Order O>
	void canonicalize(const grid_span_t<O> orig_grid) {
		// TODO assert that is a complete, valid sudoku?
		canon_label<O>(orig_grid);
		canon_place<O>(orig_grid);
	}


	template<class T>
	requires std::is_integral_v<T>
	void canonicalize(Order order, std::span<T> grid) {
		assert(is_order_compiled(order));
		assert(grid.size() >= order*order*order*order);
		switch (order) {
		#define M_SOLVENT_TEMPL_TEMPL(O_) \
			case O_: { \
				constexpr unsigned O4 = O_*O_*O_*O_; \
				using val_t = size<O_>::ord2i_t; \
				std::array<val_t,O4> grid_resize; \
				for (unsigned i {0}; i < O4; ++i) { grid_resize[i] = static_cast<val_t>(grid[i]); } \
				canonicalize<O_>(std::span(grid_resize)); \
				for (unsigned i {0}; i < O4; ++i) { grid[i] = static_cast<T>(grid_resize[i]); } \
				break; \
			}
		M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef M_SOLVENT_TEMPL_TEMPL
		}
	}

	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		extern template void canon_label<O_>(grid_span_t<O_>); \
		extern template void canon_place<O_>(grid_span_t<O_>); \
		template void canonicalize<O_>(grid_span_t<O_>);
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL

	template void canonicalize<size<O_MAX>::ord2i_t>(Order O, std::span<size<O_MAX>::ord2i_t>);
}