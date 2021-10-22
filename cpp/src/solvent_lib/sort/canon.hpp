#ifndef HPP_SOLVENT_LIB_SORT_CANON
#define HPP_SOLVENT_LIB_SORT_CANON

#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>

#include <iosfwd>
#include <vector>

namespace solvent::lib::canon {

	// input must be a complete grid.
	template<Order O>
	void canonicalize(std::vector<typename size<O>::ord2_t>& input) noexcept;


	#define SOLVENT_TEMPL_TEMPL(O_) \
		extern template void canonicalize<O_>(std::vector<typename size<O_>::ord2_t>&) noexcept;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}
#endif