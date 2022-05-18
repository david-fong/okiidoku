#include <okiidoku/puzzle/solver/cand_elim_apply.hpp>

#include <algorithm>

namespace okiidoku::mono::detail::solver {

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


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class CandElimApply<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}