#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_APPLY
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_APPLY

#include <okiidoku/puzzle/solver/engine.hpp>
#include <okiidoku/puzzle/solver/found.hpp>

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	class CandElimApply final {
		using T = Ints<O>;
		using o2x_t = int_ts::o2x_t<O>;
		using o2i_t = int_ts::o2i_t<O>;
		using rmi_t = int_ts::o4xs_t<O>;

	public:
		static SolutionsRemain apply(Engine<O>&, const found::CellClaimSym<O>&) noexcept;

		static SolutionsRemain apply(Engine<O>&, const found::SymClaimCell<O>&) noexcept;

		// AKA "naked subsets"
		static SolutionsRemain apply(Engine<O>&, const found::CellsClaimSyms<O>&) noexcept;

		// AKA "hidden subsets"
		static SolutionsRemain apply(Engine<O>&, const found::SymsClaimCells<O>&) noexcept;

		static SolutionsRemain apply(Engine<O>&, const found::LockedCands<O>&) noexcept;

		// static SolutionsRemain apply(Engine<O>&, const found::&) noexcept;
	};
}
#endif