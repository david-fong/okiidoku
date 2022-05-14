#ifndef HPP_OKIIDOKU__PUZZLE__SOLVE
#define HPP_OKIIDOKU__PUZZLE__SOLVE

#include <okiidoku/grid.hpp>
#include <okiidoku/house_mask.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <optional>
#include <memory> // unique_ptr
#if __has_include(<experimental/propagate_const>)
#include <experimental/propagate_const>
#endif

namespace okiidoku::mono::puzzle {

	template<Order O> requires(is_order_compiled(O))
	class OKIIDOKU_EXPORT FastSolver final {
	public:
		// contract: none. puzzle can even blatantly break the one rule,
		explicit FastSolver(const Grid<O>& puzzle) noexcept;

		~FastSolver() noexcept;

		// disallow copies. not much reason.
		// I just don't see it being needed. there's a lot of state too.
		// Note: already implicitly deleted due to unique_ptr field.
		FastSolver(const FastSolver&) = delete;
		FastSolver& operator=(const FastSolver&) = delete;

		// allow moves. using a moved-from `FastSolver` is UB.
		FastSolver(FastSolver&&) = default;
		FastSolver& operator=(FastSolver&&) = default;

		// return of `std::nullopt` means no more solutions exist for the puzzle.
		// example usage: `while (const auto solution {solution_walker.get_next_solution()}; solution) {...}`
		[[nodiscard]] std::optional<Grid<O>> get_next_solution() noexcept;

	private:
		/* OKIIDOKU_NO_EXPORT */ class Impl;
		#if __has_include(<experimental/propagate_const>)
		std::experimental::propagate_const<std::unique_ptr<Impl>> impl_;
		#else // fallback for MSVC
		std::unique_ptr<Impl> impl_;
		#endif
	};
}


namespace okiidoku::visitor::puzzle {
}
#endif