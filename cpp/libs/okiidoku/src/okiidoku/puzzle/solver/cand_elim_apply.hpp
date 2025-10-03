// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_APPLY
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_APPLY

#include <okiidoku/puzzle/solver/engine.hpp>
#include <okiidoku/puzzle/solver/found.hpp>
#include <okiidoku/puzzle/solver/cand_elim_find.macros.hpp>
#include <okiidoku/order.hpp>

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	class CandElimApply {
	public:
		/**
		\pre `engine.has_queued_cand_elims` returns `true`. */
		static UnwindInfo apply_first_queued(Engine<O>&) noexcept;

		static UnwindInfo apply_all_queued(Engine<O>&) noexcept;
	};


	template<Order O> requires(is_order_compiled(O))
	class CandElimApplyImpl {
	private:
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		// Important implementation reminder: if any candidate elimination done
		// as part of an `apply` function unwinds the guess stack, any remaining
		// eliminations that would have been done as part of the `apply` must not
		// be done- the `apply` must immediately short-circuit.

	public:
		/**
		\note takes desc by-value instead of by-reference since unlike the rest of
		the queues, the `CellClaimSym` queue can be passively updated while applying
		an element of the queue. if using by-reference, such an update could invalidate
		the referred-to desc (ex. by memcpy during resize) */
		static UnwindInfo apply(Engine<O>&, const found::CellClaimSym<O>) noexcept;

		/**
		\pre the specified symbol is a candidate-symbol at the specified rmi.
			this contract is respected by the corresponding `find` function. */
		static UnwindInfo apply(Engine<O>&, const found::SymClaimCell<O>&) noexcept;

		// AKA "naked subsets"
		static UnwindInfo apply(Engine<O>&, const found::Subset<O>&) noexcept;

		static UnwindInfo apply(Engine<O>&, const found::LockedCands<O>&) noexcept;

		// static UnwindInfo apply(Engine<O>&, const found::&) noexcept;
	};
}
#endif