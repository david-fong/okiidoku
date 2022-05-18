#ifndef HPP_OKIIDOKU__PUZZLE__SOLVE
#define HPP_OKIIDOKU__PUZZLE__SOLVE

#include <okiidoku/grid.hpp>
#include <okiidoku/detail/order_templates.hpp>

#include <optional>
#include <memory> // unique_ptr
#if __has_include(<experimental/propagate_const>)
#include <experimental/propagate_const>
#endif

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	class EngineObj /* final */;

	// template<Order O> requires(is_order_compiled(O))
	// class DynamicBuilder /* final */;
}

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	class OKIIDOKU_EXPORT FastSolver final {
	public:
		// contract: none. puzzle can even blatantly break the one rule,
		explicit FastSolver(const Grid<O>& puzzle) noexcept;

		~FastSolver() noexcept;
		FastSolver(const FastSolver&) = delete;
		FastSolver& operator=(const FastSolver&) = delete;

		// allow moves. an "empty" moved-from instance always returns no solutions.
		FastSolver(FastSolver&&) = default;
		FastSolver& operator=(FastSolver&&) = default;

		// return of `std::nullopt` means no more solutions exist for the puzzle.
		// example usage: `while (const auto solution {solution_walker.get_next_solution()}; solution) {...}`
		[[nodiscard]] std::optional<Grid<O>> get_next_solution() noexcept;

	private:
		using engine_t = detail::solver::EngineObj<O>;
		#if __has_include(<experimental/propagate_const>)
		std::experimental::propagate_const<std::unique_ptr<engine_t>> engine_;
		#else // fallback for MSVC
		std::unique_ptr<engine_t> engine_;
		#endif
	};


	template<Order O> requires(is_order_compiled(O))
	class OKIIDOKU_EXPORT VeryDeductiveSolver final {
	public:
		// contract: none. puzzle can even blatantly break the one rule,
		explicit VeryDeductiveSolver(const Grid<O>& puzzle) noexcept;

		~VeryDeductiveSolver() noexcept;
		VeryDeductiveSolver(const VeryDeductiveSolver&) = delete;
		VeryDeductiveSolver& operator=(const VeryDeductiveSolver&) = delete;

		// allow moves. an "empty" moved-from instance always returns no solutions.
		VeryDeductiveSolver(VeryDeductiveSolver&&) = default;
		VeryDeductiveSolver& operator=(VeryDeductiveSolver&&) = default;

		// return of `std::nullopt` means no more solutions exist for the puzzle.
		// example usage: `while (const auto solution {solution_walker.get_next_solution()}; solution) {...}`
		[[nodiscard]] std::optional<Grid<O>> get_next_solution() noexcept;

	private:
		using engine_t = detail::solver::EngineObj<O>;
		#if __has_include(<experimental/propagate_const>)
		std::experimental::propagate_const<std::unique_ptr<engine_t>> engine_;
		#else // fallback for MSVC
		std::unique_ptr<engine_t> engine_;
		#endif
	};
}


namespace okiidoku::visitor {
}
#endif