#ifndef HPP_OKIIDOKU__PUZZLE__SOLVE
#define HPP_OKIIDOKU__PUZZLE__SOLVE

#include <okiidoku/grid.hpp>
#include <okiidoku/house_mask.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <optional>
#include <memory> // unique_ptr
#include <experimental/propagate_const>

namespace okiidoku::mono::puzzle {

	template<Order O> requires(is_order_compiled(O))
	class OKIIDOKU_EXPORT FastSolver final {
		struct Impl;
		std::experimental::propagate_const<std::unique_ptr<Impl>> impl_;

	public:
		// contract: `puzzle` is a valid grid.
		explicit FastSolver(const Grid<O>& puzzle) noexcept;

		~FastSolver();

		// disallow copies. not much reason.
		// I just don't see it being needed. there's a lot of state too.
		// Note: already implicitly deleted due to unique_ptr field.
		FastSolver(const FastSolver&) = delete;
		FastSolver& operator=(const FastSolver&) = delete;

		// allow moves. using a moved-from `FastSolver` is UB.
		FastSolver(FastSolver&&) = default;
		FastSolver& operator=(FastSolver&&) = default;

		// once `std::nullopt` is first returned, all future calls will also return `std::nullopt`.
		// can use like: while (const auto solution {solution_walker.get_next_solution()}; solution) {...}
		std::optional<Grid<O>> get_next_solution() noexcept;
	};
}


namespace okiidoku::visitor::puzzle {
}
#endif