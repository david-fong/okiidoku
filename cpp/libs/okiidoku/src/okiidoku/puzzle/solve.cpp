#include <okiidoku/puzzle/solve.hpp>

#include <okiidoku/puzzle/solver/engine.hpp>
#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

// TODO.low if this translation unit becomes slow to compile and specific functions
// are being frequently modified, consider experimenting with explicit instantiation
// at the granularity of member functions in separate translation units.

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	FastSolver<O>::FastSolver(const Grid<O>& puzzle) noexcept:
		engine_{std::make_unique<detail::solver::Engine<O>>(puzzle)} {}

	// Note: with pimpl, can't default in header since the impl class is not a
	// complete type in the header (it is forward declared there). Defaulting
	// dtor requires a complete type.
	template<Order O> requires(is_order_compiled(O))
	FastSolver<O>::~FastSolver() noexcept = default;


	template<Order O> requires(is_order_compiled(O))
	std::optional<Grid<O>> FastSolver<O>::get_next_solution() noexcept {
		if (engine_.get() == nullptr) [[unlikely]] {
			return std::nullopt;
		}
		engine_t& e {*engine_};
		if (e.no_solutions_remain()) [[unlikely]] {
			return std::nullopt;
		}
		while (e.get_num_puzcells_remaining() > 0) [[likely]] {
			{const auto check {e.process_all_queued_cand_elims()};
				assert(!e.has_queued_cand_elims());
				if (check.no_solutions_remain()) [[unlikely]] { return std::nullopt; }
				if (e.get_num_puzcells_remaining() == 0) [[unlikely]] { break; }
			}

			using Find = detail::solver::CandElimFind<O>;
			Find::sym_claim_cell(e);  if (e.has_queued_cand_elims()) { continue; }
			// Find::locked_cands(e);     if (e.has_queued_cand_elims()) { continue; }
			Find::cells_claim_syms(e); if (e.has_queued_cand_elims()) { continue; }
			Find::syms_claim_cells(e); if (e.has_queued_cand_elims()) { continue; }
			e.push_guess(Find::good_guess_candidate(e));
		}
		return std::optional<Grid<O>>{std::in_place, e.build_solution_obj()};
	}


	template<Order O> requires(is_order_compiled(O))
	std::optional<Grid<O>> VeryDeductiveSolver<O>::get_next_solution() noexcept {
		if (engine_.get() == nullptr) [[unlikely]] {
			return std::nullopt;
		}
		engine_t& e {*engine_};
		if (e.no_solutions_remain()) [[unlikely]] {
			return std::nullopt;
		}
		while (e.get_num_puzcells_remaining() > 0) [[likely]] {
			using Find = detail::solver::CandElimFind<O>;
			const auto old_guess_stack_depth {e.get_guess_stack_depth()};
			const auto did_unwind {[&]{
				const auto new_depth {e.get_guess_stack_depth()};
				assert(new_depth <= old_guess_stack_depth);
				return new_depth < old_guess_stack_depth;
			}};
			Find::sym_claim_cell(e); if (did_unwind()) [[unlikely]] { continue; }
			// Find::locked_cands(e); if (did_unwind()) [[unlikely]] { continue; }
			// TODO call other techniques

			if (e.has_queued_cand_elims()) {
				// TODO but I want to consume the deduction that would lead to the
				//  most/technique-simplest newly-findable candidate eliminations.
				//  How to implement? seems to imply a degree of "BFS".
				{const auto check {e.process_first_queued_cand_elims()};
					if (check.no_solutions_remain()) [[unlikely]] { return std::nullopt; }
					if (e.get_num_puzcells_remaining() == 0) [[unlikely]] { break; }
				}
			} else {
				e.push_guess(Find::good_guess_candidate(e));
			}
		}
		return std::optional<Grid<O>>{std::in_place, e.build_solution_obj()};
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class FastSolver<O_>; \
		template class VeryDeductiveSolver<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor {
	;
}