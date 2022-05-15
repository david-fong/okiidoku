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
		if (impl_.get() == nullptr) {
			// should I really do this? My spec says this is UB.
			return std::nullopt;
		}
		// while (num_puzzle_cells_remaining_ != 0) {
			// for (const auto& technique : techniques) {
			// 	const auto old_num_puzzle_cells_remaining {num_puzzle_cells_remaining_};
			// 	const auto hit_unsat {technique()};
			// 	if (hit_unsat) {}
			// }
		// }
		return impl_->build_solution_obj();
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