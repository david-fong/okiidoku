#include <okiidoku/puzzle/solve.hpp>

#include <okiidoku/puzzle/cell_major_deductive_solver/engine.hpp>
#include <okiidoku/puzzle/cell_major_deductive_solver/engine.tpp>
#include <okiidoku/puzzle/cell_major_deductive_solver/techniques.tpp>

// TODO.low if this translation unit becomes slow to compile and specific functions
// are being frequently modified, consider experimenting with explicit instantiation
// at the granularity of member functions in separate translation units.

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	FastSolver<O>::FastSolver(const Grid<O>& puzzle) noexcept:
		engine_{std::make_unique<detail::cell_major_deductive_solver::EngineObj<O>>(puzzle)} {}

	// Note: with pimpl, can't default in header since the impl class is not a
	// complete type in the header (it is forward declared there). Defaulting
	// dtor requires a complete type.
	template<Order O> requires(is_order_compiled(O))
	FastSolver<O>::~FastSolver() noexcept = default;


	template<Order O> requires(is_order_compiled(O))
	std::optional<Grid<O>> FastSolver<O>::get_next_solution() noexcept {
		if (engine_) [[unlikely]] {
			return std::nullopt;
		}
		engine_t& e {*engine_};
		if (e.no_solutions_remain()) [[unlikely]] {
			return std::nullopt;
		}
		while (e.get_num_puzzle_cells_remaining() > 0) [[likely]] {
			{const auto check {e.process_all_queued_cand_elims()};
				assert(!e.has_enqueued_cand_elims());
				if (check.no_solutions_remain()) [[unlikely]] { return std::nullopt; }
				if (e.get_num_puzzle_cells_remaining() == 0) [[unlikely]] { break; }
			}

			using Tech = detail::cell_major_deductive_solver::Techniques<O>;
			Tech::find_symbol_requires_cell(e);
			if (e.has_enqueued_cand_elims()) { continue; }
			Tech::find_locked_candidates(e);
			if (e.has_enqueued_cand_elims()) { continue; }
			// TODO call other techniques.
			// TODO e.push_guess(rmi, val);
		}
		return e.build_solution_obj();
	}


	template<Order O> requires(is_order_compiled(O))
	std::optional<Grid<O>> VeryDeductiveSolver<O>::get_next_solution() noexcept {
		if (engine_) [[unlikely]] {
			return std::nullopt;
		}
		engine_t& e {*engine_};
		if (e.no_solutions_remain()) [[unlikely]] {
			return std::nullopt;
		}
		while (e.get_num_puzzle_cells_remaining() > 0) [[likely]] {
			using Tech = detail::cell_major_deductive_solver::Techniques<O>;
			Tech::find_symbol_requires_cell(e);
			Tech::find_locked_candidates(e);
			// TODO call other techniques

			if (e.has_enqueued_cand_elims()) {
				// TODO sort queue. or make queue internally always sorted.
				{const auto check {e.process_first_queued_cand_elims()};
					if (check.no_solutions_remain()) [[unlikely]] { return std::nullopt; }
					if (e.get_num_puzzle_cells_remaining() == 0) [[unlikely]] { break; }
				}
			} else {
				// TODO e.push_guess(rmi, val);
			}
		}
		return e.build_solution_obj();
	}
}
namespace okiidoku::mono {
	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class detail::cell_major_deductive_solver::EngineObj<O_>; \
		template class detail::cell_major_deductive_solver::Techniques<O_>; \
		template class FastSolver<O_>; \
		template class VeryDeductiveSolver<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor {
	;
}