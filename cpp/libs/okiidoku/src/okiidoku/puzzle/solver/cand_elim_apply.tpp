#ifndef TPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_APPLY
#define TPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_APPLY

#include <okiidoku/puzzle/solver/engine.hpp>
#include <okiidoku/puzzle/cand_elim_desc.hpp>

#include <algorithm>
#include <compare>

namespace okiidoku::mono::detail::solver {


	// disable implicit template instantiation.
	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		extern template class CandElimApply<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}
#endif