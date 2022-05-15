#include <okiidoku/puzzle/solve.hpp>
#include <okiidoku/puzzle/solver_base.tpp>

// #include <algorithm>
#include <memory> // unique_ptr

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	FastSolver<O>::FastSolver(const Grid<O>& puzzle) noexcept:
		impl_{std::make_unique<detail::cell_major_deductive_solver::LowLevelEngine<O>>(puzzle)} {}

	// Note: with pimpl, can't default in header since the impl class is not a
	// complete type in the header (it is forward declared there). Defaulting
	// dtor requires a complete type.
	template<Order O> requires(is_order_compiled(O))
	FastSolver<O>::~FastSolver() noexcept = default;


	template<Order O> requires(is_order_compiled(O))
	std::optional<Grid<O>> FastSolver<O>::get_next_solution() noexcept {
		if (impl_) [[unlikely]] {
			// should I really do this? My spec says this is UB.
			return std::nullopt;
		}
		impl_t& impl {*impl_};
		if (impl.no_solutions_remain()) [[unlikely]] {
			return std::nullopt;
		}
		{const auto check {impl.process_all_queued_commit_effects()};
		// TODO.asap annoying to have to write this unwind part, and it's almost certainly an error not to!
		// I think it makes sense for the engine to abstract the unwinding away so that
		if (hit_unsat(check) && hit_unsat(impl.unwind_and_rule_out_bad_guesses())) {
			return std::nullopt;
		}}
		while (impl.get_num_puzzle_cells_remaining() != 0) {
			{const auto check {impl.try_technique_symbol_requires_cell()};
			if (hit_unsat(check) && hit_unsat(impl.unwind_and_rule_out_bad_guesses())) {
				return std::nullopt;
			}}
			{const auto check {impl.process_all_queued_commit_effects()};
			if (hit_unsat(check) && hit_unsat(impl.unwind_and_rule_out_bad_guesses())) {
				return std::nullopt;
			}}
		}
		return impl.build_solution_obj();
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