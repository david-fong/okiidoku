#include <okiidoku/puzzle/solver/engine.hpp>

#include <algorithm>

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
	SolutionsRemain EngineObj<O>::eliminate_candidate_sym_(
		const EngineObj<O>::rmi_t rmi,
		const EngineObj<O>::val_t cand_to_elim
	) noexcept {
		auto& cell_cands {cells_cands_.at_rmi(rmi)};
		if (!cell_cands.test(cand_to_elim)) /* TODO.low likelihood */ {
			// TODO.try this if-block can technically be removed. need to benchmark to see whether it is beneficial.
			// candidate was already eliminated.
			return SolutionsRemain::yes();
		}
		const auto old_cands_count {cell_cands.count()};
		cell_cands.unset(cand_to_elim);
		const auto new_cands_count {cell_cands.count()};
		assert(new_cands_count <= old_cands_count);

		if (new_cands_count == 0) [[unlikely]] {
			return unwind_and_rule_out_bad_guesses_(*this);
		}
		if ((new_cands_count < old_cands_count) && /* TODO.low likelihood? */(new_cands_count == 1)) [[unlikely]] {
			enqueue_cand_elims_for_new_cell_requires_symbol_(rmi);
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
		// TODO.low inlined the call. seems like a latent foot-gun.
		// enqueue_cand_elims_for_new_cell_requires_symbol_(rmi);
		cand_elim_queues_.emplace(rmi, val);
		--num_puzzle_cells_remaining_;
	}
	template<Order O> requires(is_order_compiled(O))
	void EngineObj<O>::enqueue_cand_elims_for_new_cell_requires_symbol_(
		const EngineObj<O>::rmi_t rmi
	) noexcept {
		assert(num_puzzle_cells_remaining_ > 0);
		auto& cell_cands {cells_cands_.at_rmi(rmi)};
		assert(cell_cands.count() == 1);
		cand_elim_queues_.emplace(rmi, cell_cands.count_lower_zeros_assuming_non_empty_mask());
		--num_puzzle_cells_remaining_;
	}


	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain EngineObj<O>::process_first_queued_cand_elims() noexcept {
		assert(has_queued_cand_elims());
		const auto commit {cand_elim_queues_.front()};
		commit_effects_queue_.pop();
		// repetitive code. #undef-ed before end of function.
		#define OKIIDOKU_TRY_ELIM_NB_CAND \
			if (neighbour_rmi == commit.rmi) [[unlikely]] { continue; } \
			const auto check {eliminate_candidate_sym_(neighbour_rmi, commit.val)}; \
			if (check.no_solutions_remain()) [[unlikely]] { return check; }
		{
			const auto commit_row {rmi_to_row<O>(commit.rmi)};
			for (o2i_t nb_col {0}; nb_col < T::O2; ++nb_col) {
				const auto neighbour_rmi {static_cast<rmi_t>((T::O2*commit_row)+nb_col)};
				OKIIDOKU_TRY_ELIM_NB_CAND
		}	}
		{
			const auto commit_col {rmi_to_col<O>(commit.rmi)};
			for (o2i_t nb_row {0}; nb_row < T::O2; ++nb_row) {
				const auto neighbour_rmi {static_cast<rmi_t>((T::O2*nb_row)+commit_col)};
				OKIIDOKU_TRY_ELIM_NB_CAND
		}	}
		{
			const auto commit_box {rmi_to_box<O>(commit.rmi)};
			for (o2i_t nb_box_cell {0}; nb_box_cell < T::O2; ++nb_box_cell) {
				const auto neighbour_rmi {static_cast<rmi_t>(box_cell_to_rmi<O>(commit_box, nb_box_cell))};
				OKIIDOKU_TRY_ELIM_NB_CAND
		}	}
		#undef OKIIDOKU_TRY_ELIM_NB_CAND
		return SolutionsRemain::yes();
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
		const EngineObj<O>::rmi_t rmi,
		const EngineObj<O>::val_t val
	) noexcept {
		// assert(!has_queued_cand_elims()); // only a strong recommendation. not a contract.
		assert(cells_cands_.at_rmi(rmi).test(val));
		assert(cells_cands_.at_rmi(rmi).count() > 1);
		guess_stack_.emplace(cells_cands_, rmi, val);

		register_new_given_(rmi, val);
	}


	template<Order O> requires(is_order_compiled(O))
	SolutionsRemain unwind_and_rule_out_bad_guesses_(EngineObj<O>& e) noexcept {
		if (e.guess_stack_.empty()) {
			e.no_solutions_remain_ = true;
			return SolutionsRemain{false};
		}
		const auto step {std::move(e.guess_stack_.top())};
		e.guess_stack_.pop();
		e.cells_cands_ = *std::move(step.prev_cells_cands);
		return e.eliminate_candidate_sym_(step.guess_rmi, step.guess_val);
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class EngineObj<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}