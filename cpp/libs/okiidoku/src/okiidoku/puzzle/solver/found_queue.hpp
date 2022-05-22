#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__FOUND_QUEUE
#define HPP_OKIIDOKU__PUZZLE__SOLVER__FOUND_QUEUE

#include <okiidoku/puzzle/solver/found.hpp>

#include <deque>
#include <tuple>

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	struct CandElimQueues final {
	private:
		template<class T>
		using queue_t = std::deque<T, /* <backing container option> */>;

		std::tuple<
			// TODO.asap the cell-requires-symbol queue should be separately, internally maintained by the engine.
			//  for efficiency reasons, it should not be cleared along with the other ones during a clear operation,
			//  since it is implicitly added to by the engine's `eliminate_` member functions instead of by a `find`
			//  function, and so those that are enqueued by other cell-requires-symbol eliminations won't ever be re-found.
			queue_t<found::CellClaimSym<O>>,
			queue_t<found::SymClaimCell<O>>,
			queue_t<found::CellsClaimSyms<O>>,
			queue_t<found::SymsClaimCells<O>>,
			queue_t<found::LockedCands<O>>
		> tup_;

	public:
		[[nodiscard, gnu::pure]] bool is_empty() const noexcept {
			return std::apply([](const auto& ...dq){ return (... || dq.empty()); }, tup_);
		}
		void clear() noexcept {
			// TODO don't clear the one for
			return std::apply([](auto& ...dq){ (... , dq.clear()); }, tup_);
			// TODO.low consider whether resizing down is a good idea here?
		}

		void emplace(found::CellClaimSym<O>&& desc)   noexcept { std::get<queue_t<found::CellClaimSym<O>>>(tup_)  .emplace_front(std::move(desc)); }
		void emplace(found::SymClaimCell<O>&& desc)   noexcept { std::get<queue_t<found::SymClaimCell<O>>>(tup_)  .emplace_front(std::move(desc)); }
		void emplace(found::CellsClaimSyms<O>&& desc) noexcept { std::get<queue_t<found::CellsClaimSyms<O>>>(tup_).emplace_front(std::move(desc)); }
		void emplace(found::SymsClaimCells<O>&& desc) noexcept { std::get<queue_t<found::SymsClaimCells<O>>>(tup_).emplace_front(std::move(desc)); }
		void emplace(found::LockedCands<O>&& desc)    noexcept { std::get<queue_t<found::LockedCands<O>>>(tup_)   .emplace_front(std::move(desc)); }
	};
}
#endif