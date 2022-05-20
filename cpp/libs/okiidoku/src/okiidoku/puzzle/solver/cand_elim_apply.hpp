#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_APPLY
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_APPLY

#include <okiidoku/puzzle/solver/engine.hpp>
#include <okiidoku/puzzle/solver/cand_elim_desc.hpp>

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	class CandElimApply final {
		using T = Ints<O>;
		using o2x_t = typename T::o2x_t;
		using o2i_t = typename T::o2i_t;
		using rmi_t = typename T::o4x_smol_t;

	public:
		static SolutionsRemain apply(EngineObj<O>&, const cand_elim_desc::CellRequiresSymbol<O>&) noexcept;

		static SolutionsRemain apply(EngineObj<O>&, const cand_elim_desc::SymbolRequiresCell<O>&) noexcept;

		// AKA "naked subsets"
		static SolutionsRemain apply(EngineObj<O>&, const cand_elim_desc::CellsRequireSymbols<O>&) noexcept;

		// AKA "hidden subsets"
		static SolutionsRemain apply(EngineObj<O>&, const cand_elim_desc::SymbolsRequireCells<O>&) noexcept;

		static SolutionsRemain apply(EngineObj<O>&, const cand_elim_desc::LockedCandidate<O>&) noexcept;

		// static SolutionsRemain apply(EngineObj<O>&, const cand_elim_desc::&) noexcept;
	};
}
#endif