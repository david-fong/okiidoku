#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_FIND
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_FIND

#include <okiidoku/puzzle/solver/engine.hpp>

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	class CandElimFind final {
		// Note: use of class wrapper instead of individual template functions eases friending.
	public:
		// common contracts and invariants for all finders:
		// contract: `no_solutions_remain` returns `false`.
		// behaviour: immediately returns if `get_num_puzcells_remaining` returns zero.

		static UnwindInfo sym_claim_cell(Engine<O>&) noexcept;

		static UnwindInfo locked_cands(Engine<O>&) noexcept;

		static UnwindInfo subsets(Engine<O>&, int_ts::o2x_t<O> max_subset_size) noexcept;

		// static UnwindInfo fish(Engine<O>&) noexcept;


		static Guess<O> good_guess_candidate(const Engine<O>&) noexcept;
	};
}
#endif