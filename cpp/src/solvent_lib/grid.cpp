#include <solvent_lib/grid.hpp>

#include <iostream>

namespace solvent::lib {

	#define SOLVENT_TEMPL_TEMPL(O_) \
		template class Grid<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}