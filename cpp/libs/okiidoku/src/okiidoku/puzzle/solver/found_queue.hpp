// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_PUZZLE_SOLVER_FOUND_QUEUE
#define HPP_OKIIDOKU_PUZZLE_SOLVER_FOUND_QUEUE

#include <okiidoku/puzzle/solver/found.hpp>
#include <okiidoku/order.hpp>

#include <deque>
#include <tuple>
#include <type_traits>

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O)) class CandElimApply;

	template<Order O> requires(is_order_compiled(O))
	struct FoundQueues {
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
			queue_t<found::Subset<O>>,
			queue_t<found::LockedCands<O>>
		>;

	private:
		queues_t tup_;

	public:
		[[nodiscard, gnu::pure]]
		bool is_empty() const noexcept {
			return std::apply([](const auto& ...q)noexcept{ return (... && q.empty()); }, tup_);
		}

		// Note: only used when unwinding the engine's guess stack.
		void clear() noexcept {
			std::apply([](auto& ...dq)noexcept{ (... , dq.clear()); }, tup_);
			OKIIDOKU_ASSERT(is_empty());
			// TODO.low consider whether resizing down is a good idea here?
		}

		void push_back(found::CellClaimSym  <O>&& desc) noexcept { std::get<queue_t<found::CellClaimSym  <O>>>(tup_).emplace_back(std::move(desc)); }
		void push_back(found::SymClaimCell  <O>&& desc) noexcept { std::get<queue_t<found::SymClaimCell  <O>>>(tup_).emplace_back(std::move(desc)); }
		void push_back(found::Subset<O>&& desc) noexcept { std::get<queue_t<found::Subset<O>>>(tup_).emplace_back(std::move(desc)); }
		void push_back(found::LockedCands   <O>&& desc) noexcept { std::get<queue_t<found::LockedCands   <O>>>(tup_).emplace_back(std::move(desc)); }
	};


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		extern template struct FoundQueues<(O_)>; // instantiated in engine.cpp
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}
#endif