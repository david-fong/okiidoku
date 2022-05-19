#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_FIND
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_FIND

#include <okiidoku/puzzle/solver/engine.hpp>
#include <okiidoku/puzzle/solver/cand_elim_desc.hpp>

namespace okiidoku::mono::detail::solver {

	// TODO.asap consider wrapping in a namespace instead of a class.
	//  The only reason I'm using a class is as a trick to share common type
	//  aliases with all the member functions. The "sharing" could alternatively
	//  be done via the prelude macro, albeit probably requiring some [[maybe_unused]] attributes.
	template<Order O> requires(is_order_compiled(O))
	class CandElimFind final {

		// Internal (somewhat obvious) contract:
		// finders must never incorrectly progress in solving a proper puzzle.

	public:
		// common contracts and invariants for all finders:
		// contract: `no_solutions_remain` returns `false`.
		// behaviour: immediately returns if `get_num_puzzle_cells_remaining` returns zero.

		// TODO.high for finders that are not (relatively) trivial to perform a full scan of the grid,
		//   provide a parameter enum `SearchEffort { find_first, find_all, };`
		static void symbol_requires_cell(EngineObj<O>&) noexcept;

		static void locked_candidates(EngineObj<O>&) noexcept;

		// AKA "naked subsets"
		static void cells_require_symbols(EngineObj<O>&) noexcept;

		// AKA "hidden subsets"
		static void symbols_require_cells(EngineObj<O>&) noexcept;

		// static void fish(EngineObj<O>&) noexcept;


		static typename EngineObj<O>::Guess good_guess_candidate(const EngineObj<O>&) noexcept;
	};
}
#endif