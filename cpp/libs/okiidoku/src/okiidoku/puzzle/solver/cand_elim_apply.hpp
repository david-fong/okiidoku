#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_APPLY
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_APPLY

#include <okiidoku/puzzle/solver/engine.hpp>
#include <okiidoku/puzzle/solver/found.hpp>

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	class CandElimApply final {
	public:
		// contract: `engine.has_queued_cand_elims` returns `true`.
		static UnwindInfo apply_first_queued(Engine<O>&) noexcept;

		static UnwindInfo apply_all_queued(Engine<O>&) noexcept;
	};


	template<Order O> requires(is_order_compiled(O))
	class CandElimApplyImpl final {
	private:
		using T = Ints<O>;
		using o1i_t = int_ts::o1i_t<O>;
		using o2x_t = int_ts::o2x_t<O>;
		using o2i_t = int_ts::o2i_t<O>;
		using o3i_t = int_ts::o3i_t<O>;
		using rmi_t = int_ts::o4xs_t<O>;
		// Important implementation reminder: if any candidate elimination done
		// as part of an `apply` function unwinds the guess stack, any remaining
		// eliminations that would have been done as part of the `apply` must not
		// be done- the `apply` must immediately short-circuit.

	public:
		// Note: takes desc by-value instead of by-reference since unlike the rest of
		// the queues, the CellClaimSym queue can be passively updated while applying
		// an element of the queue. If using by-reference, such an update could invalidate
		// the referred-to desc (ex. by memcpy during resize)
		static UnwindInfo apply(Engine<O>&, const found::CellClaimSym<O>) noexcept;

		// contract: the specified symbol is a candidate-symbol at the specified rmi.
		//  this contract is respected by the corresponding `find` function.
		static UnwindInfo apply(Engine<O>&, const found::SymClaimCell<O>&) noexcept;

		// AKA "naked subsets"
		static UnwindInfo apply(Engine<O>&, const found::Subset<O>&) noexcept;

		static UnwindInfo apply(Engine<O>&, const found::LockedCands<O>&) noexcept;

		// static UnwindInfo apply(Engine<O>&, const found::&) noexcept;
	};
}
#endif