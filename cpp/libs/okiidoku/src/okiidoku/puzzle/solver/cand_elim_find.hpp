// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_FIND
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_FIND

#include <okiidoku/puzzle/solver/engine.hpp>
#include <okiidoku/puzzle/solver/cand_elim_find.macros.hpp>

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	class CandElimFind {
		// Note: use of class wrapper instead of individual template functions eases friending.
	public:
		// common contracts and invariants for all finders:
		// contract: `no_more_solns` returns `false`.
		// behaviour: immediately returns if `get_num_unsolved` returns zero.

		static UnwindInfo sym_claim_cell(Engine<O>&) noexcept;

		static UnwindInfo subsets(Engine<O>&, Ints<O>::o2x_t max_subset_size) noexcept;

		static UnwindInfo locked_cands(Engine<O>&) noexcept;

		static UnwindInfo fish(Engine<O>&) noexcept;


		static Guess<O> good_guess_candidate(const Engine<O>&) noexcept;

		// static UnwindInfo common_post_guess_cell_claim_sym(Engine<O>&);
	};
}
#endif