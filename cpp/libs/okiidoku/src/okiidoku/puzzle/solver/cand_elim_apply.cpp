// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/puzzle/solver/cand_elim_apply.hpp>

#include "okiidoku/puzzle/solver/engine.hpp"      // UnwindInfo, Engine...
#include "okiidoku/puzzle/solver/found_queue.hpp" // FoundQueues
#include "okiidoku/puzzle/solver/found.hpp"       // CellClaimSym, Lock...
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <algorithm>
#include <tuple>       // apply, get
#include <type_traits> // decay_t, is_same_v
#include <utility>     // move, tuple_element_t

namespace okiidoku::mono::detail::solver {

	namespace {
		constexpr bool logical_and_loop_continue {true};
		constexpr bool logical_and_loop_break {false};

		template<Order O, class QueueT> requires(is_order_compiled(O))
		void queue_apply_one(Engine<O>& engine, QueueT& queue, UnwindInfo& check) noexcept {
			OKIIDOKU_CONTRACT_USE(!queue.empty());
			if constexpr (std::is_same_v<std::decay_t<decltype(queue)>, typename FoundQueues<O>::template queue_t<found::CellClaimSym<O>>>) {
				auto desc {std::move(queue.front())}; // handle passive find during apply
				queue.pop_front();
				check = CandElimApplyImpl<O>::apply(engine, std::move(desc));
			} else {
				#ifndef NDEBUG
				const auto old_front_addr {&queue.front()};
				#endif
				check = CandElimApplyImpl<O>::apply(engine, queue.front());
				if (!check.did_unwind()) [[likely]] {
					OKIIDOKU_CONTRACT_ASSERT(old_front_addr == &queue.front()); // no passive find during apply
					OKIIDOKU_CONTRACT_USE(!queue.empty());
					queue.pop_front();
				} else {
					OKIIDOKU_CONTRACT_ASSERT(queue.empty());
				}
			}
		}
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo CandElimApply<O>::apply_first_queued(Engine<O>& engine) noexcept {
		OKIIDOKU_CONTRACT_ASSERT(engine.has_queued_cand_elims());
		auto check {UnwindInfo::make_no_unwind()};
		// Note: I had to choose between easier-to-understand code, or making it
		// impossible to forget to update this "boilerplate" if I implement more
		// solving techniques. I chose the latter (sorry?).

		// MSVC 19 seems to require logical_and_loop_body to be an lvalue for some reason.
		const auto logical_and_loop_body {[&](auto& queue) -> bool {
			if (queue.empty()) { return logical_and_loop_continue; }
			queue_apply_one(engine, queue, check);
			return logical_and_loop_break;
		}};
		std::apply([&](auto& ...queue){
			// see https://en.cppreference.com/w/cpp/language/eval_order
			return (... && logical_and_loop_body(queue));
		}, engine.get_found_queues_().tup_);
		return check;
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo CandElimApply<O>::apply_all_queued(Engine<O>& engine) noexcept {
		auto check {UnwindInfo::make_no_unwind()};
		const auto logical_and_loop_body {[&](auto& queue) -> bool {
			while (!queue.empty()) {
				queue_apply_one(engine, queue, check);
				if (check.did_unwind_root()) { return logical_and_loop_break; }
			}
			return logical_and_loop_continue;
		}};
		std::apply([&](auto& ...queue){
			return (... && logical_and_loop_body(queue));
		}, engine.get_found_queues_().tup_);
		if (check.did_unwind_root()) [[unlikely]] { return check; }

		using queues_t = typename FoundQueues<O>::queues_t;
		using last_queue_t = std::tuple_element_t<std::tuple_size<queues_t>()-1u, queues_t>;
		using passive_queue_t = typename FoundQueues<O>::template queue_t<found::CellClaimSym<O>>;
		if constexpr (!std::is_same_v<last_queue_t, passive_queue_t>) {
			logical_and_loop_body(std::get<passive_queue_t>(engine.get_found_queues_().tup_));
		}
		OKIIDOKU_CONTRACT_ASSERT(!engine.has_queued_cand_elims());
		return check;
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo CandElimApplyImpl<O>::apply(
		Engine<O>& engine,
		const found::CellClaimSym<O> desc
	) noexcept {
		// repetitive code. #undef-ed before end of function.
		#define OKIIDOKU_TRY_ELIM_NB_CAND \
			if (nb_rmi == desc.rmi) [[unlikely]] { continue; } \
			const auto check {engine.do_elim_remove_sym_(nb_rmi, desc.sym)}; \
			if (check.did_unwind()) [[unlikely]] { return check; }

		// The "unrolled" version is ~3% faster for O=3 :/
		{
			const auto desc_row {rmi_to_row<O>(desc.rmi)};
			for (const auto nb_col : T::O2) {
				const auto nb_rmi {static_cast<rmi_t>(row_col_to_rmi<O>(desc_row, nb_col))};
				OKIIDOKU_TRY_ELIM_NB_CAND
		}	}
		{
			const auto desc_col {rmi_to_col<O>(desc.rmi)};
			for (const auto nb_row : T::O2) {
				const auto nb_rmi {static_cast<rmi_t>(row_col_to_rmi<O>(nb_row, desc_col))};
				OKIIDOKU_TRY_ELIM_NB_CAND
		}	}
		{
			const auto desc_box {rmi_to_box<O>(desc.rmi)};
			for (const auto nb_box_cell : T::O2) {
				const auto nb_rmi {static_cast<rmi_t>(box_cell_to_rmi<O>(desc_box, nb_box_cell))};
				OKIIDOKU_TRY_ELIM_NB_CAND
		}	}
		// for (const auto house_type : house_types) {
		// 	const auto desc_house {rmi_to_house<O>(house_type, desc.rmi)};
		// 	for (const auto nb_house_cell : T::O2) {
		// 		const auto nb_rmi {house_cell_to_rmi<O>(house_type, desc_house, nb_house_cell)};
		// 		OKIIDOKU_TRY_ELIM_NB_CAND
		// 	}
		// }
		#undef OKIIDOKU_TRY_ELIM_NB_CAND
		return UnwindInfo::make_no_unwind();
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo CandElimApplyImpl<O>::apply(
		Engine<O>& engine,
		const found::SymClaimCell<O>& desc
	) noexcept {
		const auto& cell_cands {engine.cells_cands()[desc.rmi]};
		if (!cell_cands[desc.sym]) [[unlikely]] {
			return engine.unwind_one_stack_frame();
		}
		if (cell_cands.count() > 1u) [[likely]] {
			engine.register_new_given_(desc.rmi, desc.sym);
		}
		return UnwindInfo::make_no_unwind();
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo CandElimApplyImpl<O>::apply(
		Engine<O>& engine,
		const found::Subset<O>& desc
	) noexcept {
		(void)engine; (void)desc;
		return UnwindInfo::make_no_unwind();
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo CandElimApplyImpl<O>::apply(
		Engine<O>& engine,
		const found::LockedCands<O>& desc
	) noexcept {
		/** first isec-rmi of first isec in target house */
		const o3x1_t isec_base {[&]{
			switch (desc.remove_from_rest_of) {
				using enum BoxOrLine;
				case box:  return o3x1_t{((desc.isec/T::O2)*T::O2)+(desc.isec%T::O1)};
				case line: return o3x1_t{(desc.isec/T::O1)*T::O1};
			}
			OKIIDOKU_UNREACHABLE;
		}()};
		const o1i_t neighbour_scale {[&][[gnu::pure]]{
			switch (desc.remove_from_rest_of) {
				using enum BoxOrLine;
				case box:  return T::O1;
				case line: return o1i_t{1u};
			}
			OKIIDOKU_UNREACHABLE;
		}()};
		for (const auto house_isec_i : T::O1) {
			const auto isec {o3x_t::unchecked_from(isec_base + (house_isec_i * neighbour_scale))};
			if (isec == desc.isec) [[unlikely]] { continue; }
			for (const auto isec_cell_i : T::O1) {
				const auto chute {isec / T::O2};
				OKIIDOKU_CONTRACT_ASSERT(chute == desc.isec/T::O2);
				const auto chute_cell {((isec%T::O2)*T::O1) + isec_cell_i};
				const auto rmi {chute_cell_to_rmi<O>(desc.line_type, chute, chute_cell)};
				const auto check {engine.do_elim_remove_syms_(rmi, desc.syms)};
				if (check.did_unwind()) [[unlikely]] {
					return check;
				}
			}
		}
		return UnwindInfo::make_no_unwind();
	}


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template class CandElimApply<(O_)>; \
		template class CandElimApplyImpl<(O_)>;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}