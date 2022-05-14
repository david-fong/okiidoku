#include <okiidoku/puzzle/solve.hpp>
#include <okiidoku/puzzle/solver_base.tpp>

#include <algorithm>

namespace okiidoku::mono::puzzle {

	template<Order O> requires(is_order_compiled(O))
	FastSolver<O>::FastSolver(const Grid<O>& puzzle) noexcept:
		impl_{std::make_unique<detail::SolverBase<O>>(puzzle)} {}


	// Note: with pimpl, can't default in header since the impl class is not a
	// complete type in the header (it is forward declared there). Defaulting
	// dtor requires a complete type.
	template<Order O> requires(is_order_compiled(O))
	FastSolver<O>::~FastSolver() noexcept = default;


	template<Order O> requires(is_order_compiled(O))
	std::optional<Grid<O>> FastSolver<O>::get_next_solution() noexcept {
		return impl_->get_next_solution();
	}


	// Note: no need to explicitly instantiate Impl
	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class FastSolver<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor::puzzle {
	;
}