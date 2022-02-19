#include <solvent_lib/morph/analyze.hpp>

namespace solvent::lib::morph {

	template<Order O>
	grid_vec_t<O> analyze(const grid_vec_t<O>& input) {
		// TODO assert that input is the correct length and is a complete, valid sudoku?
		GridInnerRelStats<O> stats(input);
		return stats();
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template struct GridStats<O_>; \
		template grid_vec_t<O> analyze<O_>(const grid_vec_t<O_>&); \
		template class GridInnerRelStats<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}