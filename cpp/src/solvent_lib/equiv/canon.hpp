#ifndef HPP_SOLVENT_LIB__EQUIV__CANON
#define HPP_SOLVENT_LIB__EQUIV__CANON

#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

#include <vector>
#include <array>
#include <compare>

namespace solvent::lib::equiv {

	/**
	input must be a complete grid.
	See readme for information about the algorithm.
	*/
	template<Order O>
	[[gnu::const]] grid_vec_t<O> canonicalize(grid_vec_t<O> const& input);


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template grid_vec_t<O_> canonicalize<O_>(grid_vec_t<O_> const&); \
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif