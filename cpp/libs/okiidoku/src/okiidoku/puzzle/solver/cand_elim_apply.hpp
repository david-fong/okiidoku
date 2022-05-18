#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_APPLY
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_APPLY

#include <okiidoku/puzzle/solver/engine.hpp>
#include <okiidoku/puzzle/solver/cand_elim_desc.hpp>

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	class CandElimApply final {

		static SolutionsRemain symbol_requires_cell(EngineObj<O>&, const cand_elim_desc::SymbolRequiresCell<O>&) noexcept;

		// AKA "naked subsets"
		static SolutionsRemain cells_require_symbols(EngineObj<O>&, const cand_elim_desc::CellsRequireSymbols<O>&) noexcept;

		// AKA "hidden subsets"
		static SolutionsRemain symbols_require_cells(EngineObj<O>&, const cand_elim_desc::SymbolsRequireCells<O>&) noexcept;

		static SolutionsRemain locked_candidates(EngineObj<O>&, const cand_elim_desc::LockedCandidate<O>&) noexcept;

		// static SolutionsRemain fish(EngineObj<O>&, const cand_elim_desc::&) noexcept;
	};


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		extern template class CandElimApply<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}
#endif