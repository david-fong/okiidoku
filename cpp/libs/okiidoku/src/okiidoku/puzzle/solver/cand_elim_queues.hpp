#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_QUEUES
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_QUEUES

#include <okiidoku/puzzle/solver/cand_elim_desc.hpp>

#include <deque>
#include <tuple>

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	struct CandElimQueues final {
		std::tuple<
			std::deque<cand_elim_desc::subsets::CellRequiresSymbol<O>>,
			std::deque<cand_elim_desc::subsets::SymbolRequiresCell<O>>,
			std::deque<cand_elim_desc::subsets::CellsRequireSymbols<O>>,
			std::deque<cand_elim_desc::subsets::SymbolsRequireCells<O>>,
			std::deque<cand_elim_desc::LockedCandidate<O>>,
		> deques_;

		[[nodiscard, gnu::pure]] bool empty() const noexcept {
			return false; // TODO.asap
		}
	};
}
#endif