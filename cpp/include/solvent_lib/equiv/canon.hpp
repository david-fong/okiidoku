#ifndef HPP_SOLVENT_LIB__EQUIV__CANON
#define HPP_SOLVENT_LIB__EQUIV__CANON

#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

#include <array>

namespace solvent::lib::equiv {

	/**
	input must be a complete grid.
	See readme for information about the algorithm.
	*/
	template<Order O>
	[[gnu::const]] grid_vec_t<O> canonicalize(const grid_vec_t<O>& input);


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template grid_vec_t<O_> canonicalize<O_>(const grid_vec_t<O_>&); \
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif