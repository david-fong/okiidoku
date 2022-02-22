#include <solvent_lib/morph/canon.hpp>
#include <solvent_lib/print.hpp> // TODO remove after done implementing

#include <iostream>

namespace solvent::lib::morph {

	template<Order O>
	grid_vec_t<O> canon_label(grid_const_span_t<O>);

	template<Order O>
	grid_vec_t<O> canon_place(const grid_vec_t<O>& input);

	template<Order O>
	grid_vec_t<O> canonicalize(const grid_vec_t<O>& input) {
		// TODO assert that input is the correct length and is a complete, valid sudoku?
		grid_vec_t<O> out = canon_label<O>(grid_const_span_t<O>(input));
		return out;
	}

	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template grid_vec_t<O_> canon_label<O_>(grid_const_span_t<O_>); \
		extern template grid_vec_t<O_> canon_place<O_>(const grid_vec_t<O_>&); \
		template grid_vec_t<O_> canonicalize<O_>(const grid_vec_t<O_>&);
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}