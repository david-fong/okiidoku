#ifndef TPP_OKIIDOKU__PUZZLE__CELL_MAJOR_DEDUCTIVE_SOLVER__ENGINE__TECHNIQUES
#define TPP_OKIIDOKU__PUZZLE__CELL_MAJOR_DEDUCTIVE_SOLVER__ENGINE__TECHNIQUES

#include <okiidoku/puzzle/cell_major_deductive_solver/engine.hpp>

#include <algorithm>

namespace okiidoku::mono::detail::cell_major_deductive_solver {

	template<Order O> requires(is_order_compiled(O))
	TryTechniqueResult LowLevelEngine<O>::try_technique_symbol_requires_cell() noexcept {
		assert(!no_solutions_remain());
		if (get_num_puzzle_cells_remaining() == 0) {
			return TryTechniqueResult::no_match;
		}
		// TODO.asap for each house of all house-types, check if any symbol only has one candidate-house-cell.
		// how to use masks to optimize? have an accumulator candidate-symbol mask "<house-type>_seen_cand_syms" that starts as zeros.
		// have a <house-type>_syms_with_multiple_cand_cells accumulator that also starts as zeros.
		// for each cell in the house, OR= the <house-type>_syms_with_multiple_cand_cells with the AND of <house-type>_seen_cand_syms and the cell's candidate-symbols.
		//  for bits that have changed value, remove the corresponding entry from <house-type>_single_cand_house_cell_for_sym.
		// then OR= <house-type>_seen_cand_syms with the cell's candidate-symbols.
		//  for true bits that are not also true in <house-type>_syms_with_multiple_cand_cells, add the house-cell index to the corresponding entry of <house-type>_seen_cand_syms.

		// for any matches, _if the cell is not already committed (ie. if the cell still has more than one candidate-symbols),
		//  call register_new_given_();

		return TryTechniqueResult::no_match;
	}
}
#endif