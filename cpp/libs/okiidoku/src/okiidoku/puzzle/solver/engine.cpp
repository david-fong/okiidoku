#include <okiidoku/puzzle/solver/engine.hpp>

// #ifndef NDEBUG
#include <algorithm> // count_if
#include <execution>
// #endif

#include <iostream> // TODO delete

// TODO.low go through and see where it makes sense to add [[likely/unlikely]].
//  also, many current annotations are based on a _wrong_ assumption of no guessing,
//  the assumption was naively made upon the _merit_ of optimizing upon the assumption
//  that an input puzzle has at least one solution- I forgot about guesses being a thing.
//  It would be good to actually profile instead.
namespace okiidoku::mono::detail::solver {


	template<Order O> requires(is_order_compiled(O))
	void EngineImpl<O>::reinit_with_puzzle(const Grid<O>& puzzle) noexcept {
		num_puzcells_remaining_ = T::O4;
		cells_cands_.get_underlying_array().fill(O2BitArr_ones<O>);
		found_queues_.clear();
		for (const auto house_type : house_types) {
		for (o2i_t house {0}; house < T::O2; ++house) {
			auto& subs {houses_subsets_[static_cast<unsigned char>(house_type)][house]};
			for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
				subs.rmi[house_cell] = static_cast<rmi_t>(house_cell_to_rmi<O>(house_type, house, house_cell));
			}
			subs.is_begin.unset_all();
			subs.is_begin.set(0);
		}}
		// while (!guess_stack_.empty()) { guess_stack_.pop(); }
		guess_stack_.clear();
		total_guesses_ = 0;
		no_solutions_remain_ = false;

		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			const auto& val {puzzle.at_rmi(rmi)};
			assert(val <= T::O2);
			if (val < T::O2) [[likely]] {
				register_new_given_(static_cast<rmi_t>(rmi), static_cast<val_t>(val));
			}
		}
		// debug_print_cells_cands_();
	}


	template<Order O> requires(is_order_compiled(O))
	Grid<O> EngineImpl<O>::build_solution_obj() const noexcept {
		assert(!no_solutions_remain());
		assert(get_num_puzcells_remaining() == 0);
		Grid<O> soln;
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			const auto& cell_cands {cells_cands().at_rmi(rmi)};
			assert(cell_cands.count() == 1);
			soln.at_rmi(rmi) = cell_cands.count_lower_zeros_assuming_non_empty_mask();
		}
		assert(grid_is_filled(soln));
		assert(grid_follows_rule(soln));
		return soln;
	}


	template<Order O> requires(is_order_compiled(O))
	template<class F> requires(std::is_invocable_v<F, O2BitArr<O>&>)
	UnwindInfo EngineImpl<O>::do_elim_generic_(
		const EngineImpl<O>::rmi_t rmi,
		F elim_fn
	) noexcept {
		assert(!no_solutions_remain());
		auto& cell_cands {cells_cands_.at_rmi(rmi)};
		const auto old_cands_count {cell_cands.count()};

		elim_fn(cell_cands);

		const auto new_cands_count {cell_cands.count()};
		assert(new_cands_count <= old_cands_count);

		if (new_cands_count == 0) [[unlikely]] {
			return unwind_one_stack_frame_of_(*this);
		}
		if ((new_cands_count < old_cands_count) && (new_cands_count == 1)) [[unlikely]] {
			enqueue_cand_elims_for_new_cell_claim_sym_(rmi);
		}
		return UnwindInfo::make_no_unwind();
	}

	template<Order O> requires(is_order_compiled(O))
	UnwindInfo EngineImpl<O>::do_elim_remove_sym_(
		const EngineImpl<O>::rmi_t rmi,
		const EngineImpl<O>::val_t cand_to_elim
	) noexcept {
		assert(!no_solutions_remain());
		if (!cells_cands().at_rmi(rmi).test(cand_to_elim)) [[unlikely]] {
			return UnwindInfo::make_no_unwind(); // perf optimization
		}
		return do_elim_generic_(rmi, [&](auto& cands){ cands.unset(cand_to_elim); });
	}

	template<Order O> requires(is_order_compiled(O))
	UnwindInfo EngineImpl<O>::do_elim_remove_syms_(
		const EngineImpl<O>::rmi_t rmi,
		const O2BitArr<O>& to_remove
	) noexcept {
		assert(!no_solutions_remain());
		return do_elim_generic_(rmi, [&](auto& cands){ cands.remove(to_remove); });
	}

	template<Order O> requires(is_order_compiled(O))
	UnwindInfo EngineImpl<O>::do_elim_retain_syms_(
		const EngineImpl<O>::rmi_t rmi,
		const O2BitArr<O>& to_retain
	) noexcept {
		assert(!no_solutions_remain());
		return do_elim_generic_(rmi, [&](auto& cands){ cands.retain_only(to_retain); });
	}


	template<Order O> requires(is_order_compiled(O))
	void EngineImpl<O>::register_new_given_(
		const EngineImpl<O>::rmi_t rmi,
		const EngineImpl<O>::val_t val
	) noexcept {
		assert(!no_solutions_remain());
		assert(val < T::O2);
		auto& cell_cands {cells_cands_.at_rmi(rmi)};
		assert(cell_cands.test(val));
		assert(cell_cands.count() > 1);
		cell_cands.unset_all();
		cell_cands.set(val);
		assert(cell_cands.test(val));
		assert(cell_cands.count() == 1);
		enqueue_cand_elims_for_new_cell_claim_sym_(rmi);
	}


	template<Order O> requires(is_order_compiled(O))
	void EngineImpl<O>::enqueue_cand_elims_for_new_cell_claim_sym_(
		const EngineImpl<O>::rmi_t rmi
	) noexcept {
		assert(!no_solutions_remain());
		assert(get_num_puzcells_remaining() > 0);
		const auto& cell_cands {cells_cands().at_rmi(rmi)};
		const auto val {cell_cands.count_lower_zeros_assuming_non_empty_mask()};
		assert(cell_cands.test(val));
		assert(cell_cands.count() == 1);
		found_queues_.push_back(found::CellClaimSym<O>{.rmi{rmi},.val{val}});
		--num_puzcells_remaining_;
		assert(debug_check_correct_num_puzcells_remaining_());
	}


	template<Order O> requires(is_order_compiled(O))
	void EngineImpl<O>::push_guess(
		const Guess<O> guess
	) noexcept {
		assert(!no_solutions_remain());
		assert(!has_queued_cand_elims());
		assert(cells_cands_.at_rmi(guess.rmi).test(guess.val));
		assert(cells_cands_.at_rmi(guess.rmi).count() > 1);
		guess_stack_.emplace_back(*this, guess);

		register_new_given_(guess.rmi, guess.val);
		++total_guesses_;
		#ifndef NDEBUG
		std::clog << "\nguess+(" << get_guess_stack_depth() << ") " << int(guess.rmi) << " " << int(guess.val);
		// debug_print_cells_cands_();
		#endif
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo unwind_one_stack_frame_of_(EngineImpl<O>& e) noexcept {
		assert(!e.no_solutions_remain());
		e.found_queues_.clear();
		assert(!e.has_queued_cand_elims());
		if (e.guess_stack_.empty()) {
			e.no_solutions_remain_ = true;
			return UnwindInfo::make_did_unwind_root();
		}
		auto& frame {e.guess_stack_.back()};

		e.num_puzcells_remaining_ = frame.num_puzcells_remaining;

		assert(frame.prev_cells_cands.get() != nullptr);
		// e.cells_cands_ = *std::move(frame.prev_cells_cands);
		e.cells_cands_ = *std::move(frame.prev_cells_cands);

		e.houses_subsets_ = std::move(frame.houses_subsets);

		assert(e.debug_check_correct_num_puzcells_remaining_());

		auto& cell_cands {e.cells_cands_.at_rmi(frame.guess.rmi)};
		assert(cell_cands.test(frame.guess.val));
		assert(cell_cands.count() > 1);
		cell_cands.unset(frame.guess.val);
		if (cell_cands.count() == 1) [[unlikely]] {
			e.enqueue_cand_elims_for_new_cell_claim_sym_(frame.guess.rmi);
		}
		e.guess_stack_.pop_back();
		#ifndef NDEBUG
		// std::clog << "\nguess-(" << e.get_guess_stack_depth() << ") " << int(frame.guess.rmi) << " " << int(frame.guess.val);
		// e.debug_print_cells_cands_();
		#endif
		return UnwindInfo::make_did_unwind_guess();
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo Engine<O>::unwind_one_stack_frame() noexcept {
		return unwind_one_stack_frame_of_(static_cast<EngineImpl<O>&>(*this));
	}


	template<Order O> requires(is_order_compiled(O))
	void EngineImpl<O>::debug_print_cells_cands_() const noexcept {
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			if (rmi % T::O2 == 0) { std::clog << '\n'; }
			const auto& mask {cells_cands_.at_rmi(rmi)};
			const auto chars {mask.to_stringbuf()};
			std::clog.write(chars.data(), chars.size());
			std::clog << "  " << std::flush;
		}
	}

	template<Order O> requires(is_order_compiled(O))
	bool EngineImpl<O>::debug_check_correct_num_puzcells_remaining_() const noexcept {
		return get_num_puzcells_remaining() == Ints<O>::O4 - static_cast<int_ts::o4i_t<O>>(std::count_if(
			#ifdef __cpp_lib_execution
			std::execution::unseq,
			#endif
			cells_cands().get_underlying_array().cbegin(),
			cells_cands().get_underlying_array().cend(),
			[](const auto& c){ return c.count() == 1; }
		));
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template struct FoundQueues<O_>; \
		template struct EngineImpl<O_>; \
		template class Engine<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}