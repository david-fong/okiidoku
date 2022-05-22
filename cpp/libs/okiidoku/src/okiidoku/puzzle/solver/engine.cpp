#include <okiidoku/puzzle/solver/engine.hpp>

#include <algorithm>

// TODO.low go through and see where it makes sense to add [[likely/unlikely]].
namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	EngineObj<O>::EngineObj(const Grid<O>& puzzle) noexcept {
		cells_cands_.get_underlying_array().fill(house_mask_ones<O>);
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			const auto& given {puzzle.at_rmi(rmi)};
			assert(given <= T::O2);
			if (given >= T::O2) { continue; }
			register_new_given_(static_cast<rmi_t>(rmi), static_cast<val_t>(given));
		}
	}


	template<Order O> requires(is_order_compiled(O))
	Grid<O> EngineObj<O>::build_solution_obj() const noexcept {
		assert(!no_solutions_remain());
		assert(get_num_puzzle_cells_remaining() == 0);
		assert(( std::all_of(
			cells_cands_.get_underlying_array().cbegin(),
			cells_cands_.get_underlying_array().cend(),
			[](const auto& cands){ return cands.count() == 1; }
		) ));
		Grid<O> soln;
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			const auto& cell_cands {cells_cands_.at_rmi(rmi)};
			assert(cell_cands.count() == 1);
			soln.at_rmi(rmi) = cell_cands.count_lower_zeros_assuming_non_empty_mask();
		}
		return soln;
	}


	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain EngineObj<O>::cell_elim_cand_sym_(
		const EngineObj<O>::rmi_t rmi,
		const EngineObj<O>::val_t cand_to_elim
	) noexcept {
		auto& cell_cands {cells_cands_.at_rmi(rmi)};
		if (!cell_cands.test(cand_to_elim)) {
			// TODO.try this if-block can technically be removed. need to benchmark to see whether it is beneficial.
			// candidate was already eliminated.
			return SolutionsRemain::yes();
		}
		const auto old_cands_count {cell_cands.count()};
		cell_cands.unset(cand_to_elim);
		const auto new_cands_count {cell_cands.count()};
		assert(new_cands_count <= old_cands_count);

		if (new_cands_count == 0) [[unlikely]] {
			return detail_engine_unwind_and_rule_out_bad_guesses_(*this);
		}
		if ((new_cands_count < old_cands_count) && (new_cands_count == 1)) [[unlikely]] {
			enqueue_cand_elims_for_new_cell_claim_sym_(rmi);
		}
		return SolutionsRemain::yes();
	}
	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain EngineObj<O>::cell_elim_cand_syms_(
		const EngineObj<O>::rmi_t rmi,
		const HouseMask<O>& to_remove
	) noexcept {
		auto& cell_cands {cells_cands_.at_rmi(rmi)};
		const auto old_cands_count {cell_cands.count()};
		cell_cands.remove(to_remove);
		const auto new_cands_count {cell_cands.count()};
		assert(new_cands_count <= old_cands_count);

		if (new_cands_count == 0) [[unlikely]] {
			return detail_engine_unwind_and_rule_out_bad_guesses_(*this);
		}
		if ((new_cands_count < old_cands_count) && (new_cands_count == 1)) [[unlikely]] {
			enqueue_cand_elims_for_new_cell_claim_sym_(rmi);
		}
		return SolutionsRemain::yes();
	}
	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain EngineObj<O>::cell_retain_only_cand_syms_(
		const EngineObj<O>::rmi_t rmi,
		const HouseMask<O>& to_retain
	) noexcept {
		auto& cell_cands {cells_cands_.at_rmi(rmi)};
		const auto old_cands_count {cell_cands.count()};
		cell_cands.retain_only(to_retain);
		const auto new_cands_count {cell_cands.count()};
		assert(new_cands_count <= old_cands_count);

		if (new_cands_count == 0) [[unlikely]] {
			return detail_engine_unwind_and_rule_out_bad_guesses_(*this);
		}
		if ((new_cands_count < old_cands_count) && (new_cands_count == 1)) [[unlikely]] {
			enqueue_cand_elims_for_new_cell_claim_sym_(rmi);
		}
		return SolutionsRemain::yes();
	}


	template<Order O> requires(is_order_compiled(O))
	void EngineObj<O>::register_new_given_(
		const EngineObj<O>::rmi_t rmi,
		const EngineObj<O>::val_t val
	) noexcept {
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
	void EngineObj<O>::enqueue_cand_elims_for_new_cell_claim_sym_(
		const EngineObj<O>::rmi_t rmi
	) noexcept {
		assert(num_puzzle_cells_remaining_ > 0);
		auto& cell_cands {cells_cands_.at_rmi(rmi)};
		assert(cell_cands.count() == 1);
		found_queues_.emplace(found::CellClaimSym<O>{
			.rmi{rmi},
			.val{cell_cands.count_lower_zeros_assuming_non_empty_mask()}
		});
		--num_puzzle_cells_remaining_;
		assert(get_num_puzzle_cells_remaining() == static_cast<o4i_t>(std::count_if(
			cells_cands_.get_underlying_array().cbegin(),
			cells_cands_.get_underlying_array().cend(),
			[](const auto& c){ return c.count() == 1; }
		)));
	}


	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain EngineObj<O>::process_first_queued_cand_elims() noexcept {
		// (... , found_queues_); // TODO.asap
	}


	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain EngineObj<O>::process_all_queued_cand_elims() noexcept {
		while (has_queued_cand_elims()) {
			const auto check {process_first_queued_cand_elims()};
			if (check.no_solutions_remain()) {
				return check;
			}
		}
		return SolutionsRemain::yes();
	}


	template<Order O> requires(is_order_compiled(O))
	void EngineObj<O>::push_guess(
		const EngineObj<O>::Guess guess
	) noexcept {
		// assert(!has_queued_cand_elims()); // only a guideline. not a contract.
		assert(cells_cands_.at_rmi(guess.rmi).test(guess.val));
		assert(cells_cands_.at_rmi(guess.rmi).count() > 1);
		guess_stack_.emplace(cells_cands_, get_num_puzzle_cells_remaining(), guess);

		register_new_given_(guess.rmi, guess.val);
	}


	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain detail_engine_unwind_and_rule_out_bad_guesses_(EngineObj<O>& e) noexcept {
		e.found_queues_.clear();
		if (e.guess_stack_.empty()) {
			e.no_solutions_remain_ = true;
			return SolutionsRemain{false};
		}
		const auto frame {std::move(e.guess_stack_.top())};
		e.guess_stack_.pop();
		e.cells_cands_ = *std::move(frame.prev_cells_cands);
		e.num_puzzle_cells_remaining_ = frame.num_puzzle_cells_remaining;
		return e.cell_elim_cand_sym_(frame.guess.rmi, frame.guess.val);
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class EngineObj<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}