#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_APPLY
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_APPLY

#include <okiidoku/puzzle/solver/engine.hpp>
#include <okiidoku/puzzle/solver/found.hpp>

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	class CandElimApply final {
	public:
		// contract: `engine.has_queued_cand_elims` returns `true`.
		static SolutionsRemain apply_first_queued(Engine<O>&) noexcept;

		static SolutionsRemain apply_all_queued(Engine<O>&) noexcept;
	};


	template<Order O> requires(is_order_compiled(O))
	class CandElimApplyImpl final {
		using T = Ints<O>;
		using o2x_t = int_ts::o2x_t<O>;
		using o2i_t = int_ts::o2i_t<O>;
		using rmi_t = int_ts::o4xs_t<O>;

	public:
		// note: takes desc by-value instead of by-reference since unlike the rest of
		// the queues, the CellRequireSym queue can be passively updated while applying
		// an element of the queue. If using by-reference, such an update could invalidate
		// the referred-to desc (ex. by memcpy during resize)
		static SolutionsRemain apply(Engine<O>&, const found::CellClaimSym<O>) noexcept;

		// contract: the specified symbol is a candidate-symbol at the specified rmi.
		//  this contract is respected by the corresponding `find` function.
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