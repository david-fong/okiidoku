#include <okiidoku/puzzle/solve.hpp>

#include <okiidoku/puzzle/cell_major_deductive_solver/engine.hpp>
#include <okiidoku/puzzle/cell_major_deductive_solver/engine.tpp>
#include <okiidoku/puzzle/cell_major_deductive_solver/engine.techniques.tpp>

// #include <algorithm>
#include <memory> // unique_ptr

// TODO.low if this translation unit becomes slow to compile and specific functions
// are being frequently modified, consider experimenting with explicit instantiation
// at the granularity of member functions in separate translation units.

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	FastSolver<O>::FastSolver(const Grid<O>& puzzle) noexcept:
		engine_{std::make_unique<detail::cell_major_deductive_solver::LowLevelEngine<O>>(puzzle)} {}

	// Note: with pimpl, can't default in header since the impl class is not a
	// complete type in the header (it is forward declared there). Defaulting
	// dtor requires a complete type.
	template<Order O> requires(is_order_compiled(O))
	FastSolver<O>::~FastSolver() noexcept = default;


	template<Order O> requires(is_order_compiled(O))
	std::optional<Grid<O>> FastSolver<O>::get_next_solution() noexcept {
		if (engine_) [[unlikely]] {
			// TODO.low should I really do this for the caller? My spec
			// says calling from a moved-from instance is UB...
			return std::nullopt;
		}
		engine_t& e {*engine_};
		if (e.no_solutions_remain()) [[unlikely]] {
			return std::nullopt;
		}
		while (e.get_num_puzzle_cells_remaining() > 0) [[likely]] {
			// TODO.wait comparatively, the VeryDeductive solver goes through all techniques, then consumes one queued candidate elimination, and then goes through all techniques again. It only pushes a guess if it goes through all techniques and there are no queued candidate eliminations.
			{const auto check {e.process_all_queued_commit_effects()};
				if (hit_unsat(check)) { return std::nullopt; }
				if (e.get_num_puzzle_cells_remaining() == 0) [[unlikely]] { break; }
			}
			{const auto check {e.try_technique_symbol_requires_cell()};
				if (hit_unsat(check)) { return std::nullopt; }
				if (match_ok(check)) { continue; }
				// TODO.asap consider alternate design: all candidate elimination is queueable- not just the cell-requires-symbol scenario.
			}
			{const auto check {e.try_technique_locked_candidates()};
				if (hit_unsat(check)) { return std::nullopt; }
				if (match_ok(check)) { continue; }
			}
			// TODO.asap call other techniques and finish with push_guess.
		}
		return e.build_solution_obj();
	}
}
namespace okiidoku::mono {
	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class detail::cell_major_deductive_solver::LowLevelEngine<O_>; \
		template class FastSolver<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor {
	;
}