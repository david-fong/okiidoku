#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_QUEUES
#define HPP_OKIIDOKU__PUZZLE__SOLVER__CAND_ELIM_QUEUES

#include <okiidoku/puzzle/solver/cand_elim_desc.hpp>

#include <deque>
#include <tuple>

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	struct CandElimQueues final {
		std::tuple<
			std::deque<cand_elim_desc::CellRequiresSymbol<O>>,
			std::deque<cand_elim_desc::SymbolRequiresCell<O>>,
			std::deque<cand_elim_desc::CellsRequireSymbols<O>>,
			std::deque<cand_elim_desc::SymbolsRequireCells<O>>,
			std::deque<cand_elim_desc::LockedCandidate<O>>
		> tup_;

		[[nodiscard, gnu::pure]] bool is_empty() const noexcept {
			return std::apply([](const auto& ...dq){ return (... || dq.empty()); }, tup_);
		}
		void clear() noexcept {
			return std::apply([](auto& ...dq){ (... , dq.clear()); }, tup_);
			// TODO.low consider whether resizing down is a good idea here?
		}

		void emplace(cand_elim_desc::CellRequiresSymbol<O>&& desc)  noexcept { std::get<std::deque<cand_elim_desc::CellRequiresSymbol<O>>>(tup_) .emplace_front(std::move(desc)); }
		void emplace(cand_elim_desc::SymbolRequiresCell<O>&& desc)  noexcept { std::get<std::deque<cand_elim_desc::SymbolRequiresCell<O>>>(tup_) .emplace_front(std::move(desc)); }
		void emplace(cand_elim_desc::CellsRequireSymbols<O>&& desc) noexcept { std::get<std::deque<cand_elim_desc::CellsRequireSymbols<O>>>(tup_).emplace_front(std::move(desc)); }
		void emplace(cand_elim_desc::SymbolsRequireCells<O>&& desc) noexcept { std::get<std::deque<cand_elim_desc::SymbolsRequireCells<O>>>(tup_).emplace_front(std::move(desc)); }
		void emplace(cand_elim_desc::LockedCandidate<O>&& desc)     noexcept { std::get<std::deque<cand_elim_desc::LockedCandidate<O>>>(tup_)    .emplace_front(std::move(desc)); }
	};
}
#endif