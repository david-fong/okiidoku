#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__FOUND_QUEUE
#define HPP_OKIIDOKU__PUZZLE__SOLVER__FOUND_QUEUE

#include <okiidoku/puzzle/solver/found.hpp>

#include <deque>
#include <tuple>
#include <type_traits>
#include <cassert>

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O)) class CandElimApply;

	template<Order O> requires(is_order_compiled(O))
	struct FoundQueues final {
		friend class CandElimApply<O>;
	public:
		template<class T>
		using queue_t = std::deque<T/*, <backing container option>*/>;

		using queues_t = std::tuple<
			// in this cell-major engine, this queue is special- it is pushed to
			// passively by the engine instead of by a `find` function, so keep in
			// mind that consuming from other queues can result in pushes to it.
			queue_t<found::CellClaimSym<O>>,

			queue_t<found::SymClaimCell<O>>,
			queue_t<found::CellsClaimSyms<O>>,
			queue_t<found::SymsClaimCells<O>>,
			queue_t<found::LockedCands<O>>
		>;

	private:
		queues_t tup_;

	public:
		[[nodiscard, gnu::pure]]
		bool is_empty() const noexcept {
			return std::apply([](const auto& ...q){ return (... && q.empty()); }, tup_);
		}

		// Note: only used when unwinding the engine's guess stack.
		void clear() noexcept {
			std::apply([](auto& ...dq){ (... , dq.clear()); }, tup_);
			assert(is_empty());
			// TODO.low consider whether resizing down is a good idea here?
		}

		void push_back(found::CellClaimSym  <O>&& desc) noexcept { std::get<queue_t<found::CellClaimSym  <O>>>(tup_).emplace_back(std::move(desc)); }
		void push_back(found::SymClaimCell  <O>&& desc) noexcept { std::get<queue_t<found::SymClaimCell  <O>>>(tup_).emplace_back(std::move(desc)); }
		void push_back(found::CellsClaimSyms<O>&& desc) noexcept { std::get<queue_t<found::CellsClaimSyms<O>>>(tup_).emplace_back(std::move(desc)); }
		void push_back(found::SymsClaimCells<O>&& desc) noexcept { std::get<queue_t<found::SymsClaimCells<O>>>(tup_).emplace_back(std::move(desc)); }
		void push_back(found::LockedCands   <O>&& desc) noexcept { std::get<queue_t<found::LockedCands   <O>>>(tup_).emplace_back(std::move(desc)); }
	};


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		extern template struct FoundQueues<O_>; // currently instantiated in engine.cpp
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}
#endif