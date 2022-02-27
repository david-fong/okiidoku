#include <solvent_lib/morph/canon.hpp>
#include <solvent_lib/print.hpp> // TODO remove after done implementing

#include <iostream>

namespace solvent::lib::morph {

	template<Order O>
	void canon_label(grid_span_t<O>);

	template<Order O>
	void canon_place(grid_span_t<O>);

	template<Order O>
	void canonicalize(const grid_span_t<O> orig_grid) {
		// TODO assert that input is the correct length and is a complete, valid sudoku?
		canon_label<O>(orig_grid);
	}

	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		extern template void canon_label<O_>(grid_span_t<O_>); \
		extern template void canon_place<O_>(grid_span_t<O_>); \
		template void canonicalize<O_>(grid_span_t<O_>);
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}