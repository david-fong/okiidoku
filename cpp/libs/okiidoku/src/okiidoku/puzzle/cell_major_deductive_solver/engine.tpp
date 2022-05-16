#ifndef TPP_OKIIDOKU__PUZZLE__CELL_MAJOR_DEDUCTIVE_SOLVER__ENGINE
#define TPP_OKIIDOKU__PUZZLE__CELL_MAJOR_DEDUCTIVE_SOLVER__ENGINE

#include <okiidoku/puzzle/cell_major_deductive_solver/engine.hpp>
#include <okiidoku/puzzle/cell_major_deductive_solver/engine.techniques.tpp>

#include <algorithm>

namespace okiidoku::mono::detail::cell_major_deductive_solver {

	template<Order O> requires(is_order_compiled(O))
	LowLevelEngine<O>::LowLevelEngine(const Grid<O>& puzzle) noexcept {
		cells_cands_.get_underlying_array().fill(house_mask_ones<O>);
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			const auto& given {puzzle.at_rmi(rmi)};
			assert(given <= T::O2);
			if (given >= T::O2) { continue; }
			register_new_given_(static_cast<rmi_t>(rmi), static_cast<val_t>(given));
		}
	}


	template<Order O> requires(is_order_compiled(O))
	std::optional<Grid<O>> LowLevelEngine<O>::build_solution_obj() const noexcept {
		assert(!no_solutions_remain());
		assert(get_num_puzzle_cells_remaining() == 0);
		assert(( std::all_of(
			cells_cands_.get_underlying_array().cbegin(),
			cells_cands_.get_underlying_array().cend(),
			[](const auto& cands){ return cands.count() == 1; }
		) ));

		// build solution struct and return:
		std::optional<Grid<O>> soln_optional {std::in_place};
		{
			auto& soln {soln_optional.value()};
			for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
				const auto& cell_cands {cells_cands_.at_rmi(rmi)};
				assert(cell_cands.count() == 1);
				soln.at_rmi(rmi) = cell_cands.count_lower_zeros_assuming_non_empty_mask();
			}
		}
		return soln_optional;
	}


	template<Order O> requires(is_order_compiled(O))
	CandElimResult LowLevelEngine<O>::eliminate_candidate_sym_(
		const LowLevelEngine<O>::rmi_t rmi,
		const LowLevelEngine<O>::val_t cand_to_elim
	) noexcept {
		auto& cell_cands {cells_cands_.at_rmi(rmi)};
		if (!cell_cands.test(cand_to_elim)) /* TODO.low likelihood */ {
			// candidate was already eliminated.
			return CandElimResult::ok;
		}
		const auto old_cands_count {cell_cands.count()};
		cell_cands.unset(cand_to_elim);
		const auto new_cands_count {cell_cands.count()};
		assert(new_cands_count <= old_cands_count);

		if (new_cands_count == 0) [[unlikely]] {
			return CandElimResult::unsat;
		}
		if ((new_cands_count < old_cands_count) && /* TODO.low likelihood? */(new_cands_count == 1)) [[unlikely]] {
			enqueue_commit_effects_for_new_cell_requires_symbol_(rmi);
		}
		return CandElimResult::ok;
	}


	template<Order O> requires(is_order_compiled(O))
	void LowLevelEngine<O>::register_new_given_(
		const LowLevelEngine<O>::rmi_t rmi,
		const LowLevelEngine<O>::val_t val
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
		// enqueue_commit_effects_for_new_cell_requires_symbol_(rmi);
		commit_effects_queue_.emplace(rmi, val);
		--num_puzzle_cells_remaining_;
	}
	template<Order O> requires(is_order_compiled(O))
	void LowLevelEngine<O>::enqueue_commit_effects_for_new_cell_requires_symbol_(
		const LowLevelEngine<O>::rmi_t rmi
	) noexcept {
		assert(num_puzzle_cells_remaining_ > 0);
		auto& cell_cands {cells_cands_.at_rmi(rmi)};
		assert(cell_cands.count() == 1);
		commit_effects_queue_.emplace(rmi, cell_cands.count_lower_zeros_assuming_non_empty_mask());
		--num_puzzle_cells_remaining_;
	}


	template<Order O> requires(is_order_compiled(O))
	CandElimResult LowLevelEngine<O>::process_one_queued_commit_effects() noexcept {
		assert(has_enqueued_commit_effects());
		// TODO.asap do eliminate_candidate_sym_ for all same-house cells
		const auto commit {commit_effects_queue_.front()};
		commit_effects_queue_.pop();
		#define OKIIDOKU_TRY_ELIM_NB_CAND \
			if (neighbour_rmi == commit.rmi) [[unlikely]] { continue; } \
			const auto check {eliminate_candidate_sym_(neighbour_rmi, commit.val)}; \
			if (hit_unsat(check)) [[unlikely]] { \
				return CandElimResult::unsat; \
			}
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
		return CandElimResult::ok;
	}


	template<Order O> requires(is_order_compiled(O))
	CandElimResult LowLevelEngine<O>::process_all_queued_commit_effects() noexcept {
		while (has_enqueued_commit_effects()) {
			const auto check {process_one_queued_commit_effects()};
			if (check == CandElimResult::unsat) {
				return CandElimResult::unsat;
			}
		}
		return CandElimResult::ok;
	}


	template<Order O> requires(is_order_compiled(O))
	void LowLevelEngine<O>::push_guess(
		const LowLevelEngine<O>::rmi_t rmi,
		const LowLevelEngine<O>::val_t val
	) noexcept {
		assert(!has_enqueued_commit_effects());
		assert(cells_cands_.at_rmi(rmi).test(val));
		guess_stack_.emplace(std::make_unique<GuessRecord>(cells_cands_, CommitRecord{rmi, val}));

		register_new_given_(rmi, val);
	}


	template<Order O> requires(is_order_compiled(O))
	CandElimResult LowLevelEngine<O>::unwind_and_rule_out_bad_guesses_() noexcept {
		if (guess_stack_.empty()) {
			no_solutions_remain_ = true;
			return CandElimResult::unsat;
		}
		const auto check {[&]{
			const GuessRecord& step {*guess_stack_.top()};
			cells_cands_ = std::move(step.prev_cells_cands);
			return eliminate_candidate_sym_(step.committed.rmi, step.committed.val);
		}()};
		guess_stack_.pop();
		if (check == CandElimResult::unsat) [[unlikely]] {
			return unwind_and_rule_out_bad_guesses_();
		}
		return CandElimResult::ok;
	}
}
#endif