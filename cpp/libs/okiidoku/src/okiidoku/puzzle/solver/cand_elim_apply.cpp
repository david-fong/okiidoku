#include <okiidoku/puzzle/solver/cand_elim_apply.hpp>

#include <algorithm>

namespace okiidoku::mono::detail::solver {

	namespace {
		constexpr bool logical_and_loop_continue {true};
		constexpr bool logical_and_loop_break {false};

		template<Order O, class QueueT> requires(is_order_compiled(O))
		void queue_apply_one(Engine<O>& engine, QueueT& queue, UnwindInfo& check) noexcept {
			assert(!queue.empty());
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
					assert(old_front_addr == &queue.front()); // no passive find during apply
					assert(!queue.empty());
					queue.pop_front();
				} else {
					assert(queue.empty());
				}
			}
		}
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo CandElimApply<O>::apply_first_queued(Engine<O>& engine) noexcept {
		assert(engine.has_queued_cand_elims());
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
		using last_queue_t = std::tuple_element_t<std::tuple_size<queues_t>()-1, queues_t>;
		using passive_queue_t = typename FoundQueues<O>::template queue_t<found::CellClaimSym<O>>;
		if constexpr (!std::is_same_v<last_queue_t, passive_queue_t>) {
			logical_and_loop_body(std::get<passive_queue_t>(engine.get_found_queues_().tup_));
		}
		assert(!engine.has_queued_cand_elims());
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
			const auto check {engine.do_elim_remove_sym_(nb_rmi, desc.val)}; \
			if (check.did_unwind()) [[unlikely]] { return check; }

		// The "unrolled" version is ~3% faster for O=3 :/
		{
			const auto desc_row {rmi_to_row<O>(desc.rmi)};
			for (o2i_t nb_col {0}; nb_col < T::O2; ++nb_col) {
				const auto nb_rmi {static_cast<rmi_t>((T::O2*desc_row)+nb_col)};
				OKIIDOKU_TRY_ELIM_NB_CAND
		}	}
		{
			const auto desc_col {rmi_to_col<O>(desc.rmi)};
			for (o2i_t nb_row {0}; nb_row < T::O2; ++nb_row) {
				const auto nb_rmi {static_cast<rmi_t>((T::O2*nb_row)+desc_col)};
				OKIIDOKU_TRY_ELIM_NB_CAND
		}	}
		{
			const auto desc_box {rmi_to_box<O>(desc.rmi)};
			for (o2i_t nb_box_cell {0}; nb_box_cell < T::O2; ++nb_box_cell) {
				const auto nb_rmi {static_cast<rmi_t>(box_cell_to_rmi<O>(desc_box, nb_box_cell))};
				OKIIDOKU_TRY_ELIM_NB_CAND
		}	}
		// for (const auto house_type : house_types) {
		// 	const auto desc_house {rmi_to_house<O>(house_type, desc.rmi)};
		// 	for (o2i_t nb_house_cell {0}; nb_house_cell < T::O2; ++nb_house_cell) {
		// 		const auto nb_rmi {static_cast<rmi_t>(house_cell_to_rmi<O>(house_type, desc_house, nb_house_cell))};
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
		const auto& cell_cands {engine.cells_cands().at_rmi(desc.rmi)};
		if (!cell_cands.test(desc.val)) [[unlikely]] {
			return engine.unwind_one_stack_frame();
		}
		if (cell_cands.count() > 1) [[likely]] {
			engine.register_new_given_(desc.rmi, desc.val);
		}
		return UnwindInfo::make_no_unwind();
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo CandElimApplyImpl<O>::apply(
		Engine<O>& engine,
		const found::CellsClaimSyms<O>& desc
	) noexcept {
		(void)engine; (void)desc;
		/* for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
			// TODO likelihood attribute. hypothesis: desc.house_cells.count() is small. please empirically test.
			if (desc.house_cells.test(static_cast<o2x_t>(house_cell))) [[unlikely]] {
				continue;
			}
			const auto rmi {house_cell_to_rmi<O>(desc.house_type, desc.house, house_cell)};
			const auto check {engine.do_elim_remove_syms_(static_cast<rmi_t>(rmi), desc.syms)};
			if (check.did_unwind()) [[unlikely]] {
				return check;
			}
		} */
		return UnwindInfo::make_no_unwind();
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo CandElimApplyImpl<O>::apply(
		Engine<O>& engine,
		const found::SymsClaimCells<O>& desc
	) noexcept {
		(void)engine; (void)desc;
		/* for (auto walker {desc.house_cells.set_bits_walker()}; walker.has_more(); walker.advance()) {
			const auto rmi {house_cell_to_rmi<O>(desc.house_type, desc.house, walker.value())};
			const auto check {engine.do_elim_retain_syms_(static_cast<rmi_t>(rmi), desc.syms)};
			if (check.did_unwind()) [[unlikely]] {
				return check;
			}
		} */
		return UnwindInfo::make_no_unwind();
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo CandElimApplyImpl<O>::apply(
		Engine<O>& engine,
		const found::LockedCands<O>& desc
	) noexcept {
		const auto isec_base {[&]{ switch (desc.remove_from_rest_of) {
			case BoxOrLine::box:  return static_cast<o3i_t>(((desc.isec/T::O2)*T::O2)+(desc.isec%T::O1));
			case BoxOrLine::line: return static_cast<o3i_t>(desc.isec - (desc.isec%T::O1));
			default: assert(false); // std::unreachable
		} }()};
		const auto nb_scale {[&]{ switch (desc.remove_from_rest_of) {
			case BoxOrLine::box:  return static_cast<o3i_t>(T::O1);
			case BoxOrLine::line: return static_cast<o3i_t>(1);
			default: assert(false); // std::unreachable
		} }()};
		for (o1i_t nb_i {0}; nb_i < T::O1; ++nb_i) {
			const auto isec {isec_base + (nb_i * nb_scale)};
			if (isec == desc.isec) [[unlikely]] { continue; }
			for (o1i_t isec_cell_i {0}; isec_cell_i < T::O1; ++isec_cell_i) {
				const auto chute {static_cast<o1i_t>(isec/T::O2)};
				assert(chute == static_cast<o1i_t>(desc.isec/T::O2));
				const auto chute_cell {static_cast<o3i_t>(((isec*T::O1)%T::O3) + isec_cell_i)};
				const auto rmi {chute_cell_to_rmi<O>(desc.line_type, chute, chute_cell)};
				const auto check {engine.do_elim_remove_syms_(static_cast<rmi_t>(rmi), desc.syms)};
				if (check.did_unwind()) [[unlikely]] {
					return check;
				}
			}
		}
		return UnwindInfo::make_no_unwind();
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class CandElimApply<O_>; \
		template class CandElimApplyImpl<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}