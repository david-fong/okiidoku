#ifndef HPP_SOLVENT_LIB__MORPH__CANON
#define HPP_SOLVENT_LIB__MORPH__CANON

#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>
#include <solvent_config.hpp>

namespace solvent::lib::morph {

	/** input must be a complete grid. */
	template<Order O>
	void canonicalize(grid_span_t<O>);


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		extern template void canonicalize<O_>(grid_span_t<O_>);
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}
#endif