#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_FIND
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_FIND

#include <okiidoku/puzzle/solver/engine.hpp>
#include <okiidoku/puzzle/solver/found.hpp>

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	class CandElimFind final {
		// Note: use of class wrapper instead of individual template functions eases friending.
	public:
		// common contracts and invariants for all finders:
		// contract: `no_solutions_remain` returns `false`.
		// behaviour: immediately returns if `get_num_puzcells_remaining` returns zero.

		// TODO.high for finders that are not (relatively) trivial to perform a full scan of the grid,
		//   provide a parameter enum `SearchEffort { find_first, find_all, };`
		static UnwindInfo sym_claim_cell(Engine<O>&) noexcept;

		static UnwindInfo locked_cands(Engine<O>&) noexcept;

		// AKA "naked subsets"
		static UnwindInfo cells_claim_syms(Engine<O>&) noexcept;

		// AKA "hidden subsets"
		static UnwindInfo syms_claim_cells(Engine<O>&) noexcept;

		// static UnwindInfo fish(Engine<O>&) noexcept;


		static Guess<O> good_guess_candidate(const Engine<O>&) noexcept;
	};
}
#endif