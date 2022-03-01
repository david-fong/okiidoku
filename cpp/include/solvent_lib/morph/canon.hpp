#ifndef HPP_SOLVENT_LIB__MORPH__CANON
#define HPP_SOLVENT_LIB__MORPH__CANON

#include "solvent_lib/grid.hpp"
#include "solvent_lib/size.hpp"
#include "solvent_config.hpp"

namespace solvent::lib::morph {

	// contract: the span is a complete, valid grid.
	template<Order O>
	void canonicalize(grid_span_t<O>);

	// contract: T fits size<O>::ord2i_t and canonicalize<O> is compiled. also see canonicalize<O>'s contract.
	template<class T>
	requires std::is_integral_v<T>
	void canonicalize(Order O, std::span<T>);


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		extern template void canonicalize<O_>(grid_span_t<O_>);
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
#endif