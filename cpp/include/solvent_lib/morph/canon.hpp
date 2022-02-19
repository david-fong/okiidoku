#ifndef HPP_SOLVENT_LIB__MORPH__CANON
#define HPP_SOLVENT_LIB__MORPH__CANON

#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

namespace solvent::lib::morph {
	/** input must be a complete grid. */
	template<Order O> [[nodiscard]]
	[[gnu::const]] grid_vec_t<O> canonicalize(const grid_vec_t<O>& input);


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template grid_vec_t<O_> canonicalize<O_>(const grid_vec_t<O_>&);
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif