#include <okiidoku/puzzle/solver/engine.hpp>

#include <algorithm>

// TODO.low go through and see where it makes sense to add [[likely/unlikely]].
namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	Engine<O>::Engine(const Grid<O>& puzzle) noexcept: EngineImpl<O>{puzzle} {}


	template<Order O> requires(is_order_compiled(O))
	EngineImpl<O>::EngineImpl(const Grid<O>& puzzle) noexcept {
		cells_cands_.get_underlying_array().fill(house_mask_ones<O>);
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			const auto& val {puzzle.at_rmi(rmi)};
			assert(val <= T::O2);
			if (val < T::O2) {
				register_new_given_(static_cast<rmi_t>(rmi), static_cast<val_t>(val));
				assert(cells_cands().at_rmi(rmi).count() == 1);
			} else {
				assert(cells_cands().at_rmi(rmi).count() == T::O2);
			}
		}
	}


	template<Order O> requires(is_order_compiled(O))
	Grid<O> EngineImpl<O>::build_solution_obj() const noexcept {
		assert(!no_solutions_remain());
		assert(get_num_puzcells_remaining() == 0);
		assert(( std::all_of(
			cells_cands().get_underlying_array().cbegin(),
			cells_cands().get_underlying_array().cend(),
			[](const auto& cands){ return cands.count() == 1; }
		) ));
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
	template<class F> requires(std::is_invocable_v<F, HouseMask<O>&>)
	SolutionsRemain EngineImpl<O>::do_elim_generic_(
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
			return engine_unwind_guess_(*this);
		}
		if ((new_cands_count < old_cands_count) && (new_cands_count == 1)) [[unlikely]] {
			enqueue_cand_elims_for_new_cell_claim_sym_(rmi);
		}
		return SolutionsRemain::yes();
	}

	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain EngineImpl<O>::do_elim_remove_sym_(
		const EngineImpl<O>::rmi_t rmi,
		const EngineImpl<O>::val_t cand_to_elim
	) noexcept {
		assert(!no_solutions_remain());
		if (!cells_cands().at_rmi(rmi).test(cand_to_elim)) {
			// TODO.try this if-block can technically be removed. need to benchmark to see whether it is beneficial.
			// candidate was already eliminated.
			return SolutionsRemain::yes();
		}
		return do_elim_generic_(rmi, [&](auto& cands){ cands.unset(cand_to_elim); });
	}

	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain EngineImpl<O>::do_elim_remove_syms_(
		const EngineImpl<O>::rmi_t rmi,
		const HouseMask<O>& to_remove
	) noexcept {
		assert(!no_solutions_remain());
		return do_elim_generic_(rmi, [&](auto& cands){ cands.remove(to_remove); });
	}

	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain EngineImpl<O>::do_elim_retain_syms_(
		const EngineImpl<O>::rmi_t rmi,
		const HouseMask<O>& to_retain
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
		assert(get_num_puzcells_remaining() == T::O4 - static_cast<o4i_t>(std::count_if(
			cells_cands().get_underlying_array().cbegin(),
			cells_cands().get_underlying_array().cend(),
			[](const auto& c){ return c.count() == 1; }
		)));
	}


	template<Order O> requires(is_order_compiled(O))
	void EngineImpl<O>::push_guess(
		const Guess<O> guess
	) noexcept {
		assert(!no_solutions_remain());
		assert(!has_queued_cand_elims());
		assert(cells_cands_.at_rmi(guess.rmi).test(guess.val));
		assert(cells_cands_.at_rmi(guess.rmi).count() > 1);
		guess_stack_.emplace(*this, guess);

		register_new_given_(guess.rmi, guess.val);
	}


	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain engine_unwind_guess_(EngineImpl<O>& e) noexcept {
		assert(!e.no_solutions_remain());
		e.found_queues_.clear();
		if (e.guess_stack_.empty()) {
			e.no_solutions_remain_ = true;
			return SolutionsRemain{false};
		}
		auto frame {std::move(e.guess_stack_.top())};
		static_assert(std::is_same_v<decltype(frame), typename EngineImpl<O>::GuessStackFrame>);
		e.guess_stack_.pop();

		assert(frame.prev_cells_cands.get() != nullptr);
		// e.cells_cands_ = *std::move(frame.prev_cells_cands);
		e.cells_cands_ = *frame.prev_cells_cands;

		e.num_puzcells_remaining_ = frame.num_puzcells_remaining;
		assert(e.get_num_puzcells_remaining() == Ints<O>::O4 - static_cast<int_ts::o4i_t<O>>(std::count_if(
			e.cells_cands().get_underlying_array().cbegin(),
			e.cells_cands().get_underlying_array().cend(),
			[](const auto& c){ return c.count() == 1; }
		)));
		const auto& cell_cands {e.cells_cands().at_rmi(frame.guess.rmi)};
		assert(cell_cands.test(frame.guess.val));
		assert(cell_cands.count() > 1);
		return e.do_elim_remove_sym_(frame.guess.rmi, frame.guess.val);
	}


	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain Engine<O>::unwind_guess() noexcept {
		return engine_unwind_guess_(static_cast<EngineImpl<O>&>(*this));
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template struct EngineImpl<O_>; \
		template class Engine<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}