// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__PUZZLE__SOLVE
#define HPP_OKIIDOKU__PUZZLE__SOLVE

#include <okiidoku/grid.hpp>
#include <okiidoku/order.hpp>

#include <optional>
#include <memory> // unique_ptr
#if __has_include(<experimental/propagate_const>)
#include <experimental/propagate_const>
#endif

namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	class Engine;
}

namespace okiidoku::mono {

	using num_solns_found_t = unsigned long long;


	template<Order O> requires(is_order_compiled(O))
	class OKIIDOKU_EXPORT FastSolver {
	public:
		FastSolver() noexcept;
		~FastSolver() noexcept;

		struct [[gnu::designated_init]] CandSymToIgnore {
			int_ts::o4xs_t<O> rmi;
			int_ts::o2x_t<O> val;
		};

		/// \pre none. puzzle can even blatantly break the one rule.
		void reinit_with_puzzle(const Grid<O>& puzzle, const std::optional<CandSymToIgnore> = {}) noexcept;

		// return of `std::nullopt` means no more solutions exist for the puzzle.
		// example usage: `while (const auto solution {solution_walker.get_next_solution()}; solution) {...}`
		[[nodiscard]] std::optional<Grid<O>> get_next_solution() noexcept;

		[[nodiscard, gnu::pure]]
		num_solns_found_t num_solns_found() const noexcept { return num_solns_found_; }

		FastSolver(const FastSolver&) = delete;
		FastSolver& operator=(const FastSolver&) = delete;

		// allow moves. an "empty" moved-from instance always returns no solutions.
		FastSolver(FastSolver&&) noexcept = default;
		FastSolver& operator=(FastSolver&&) noexcept = default;

	private:
		using engine_t = detail::solver::Engine<O>;
		#if __has_include(<experimental/propagate_const>)
		std::experimental::propagate_const<std::unique_ptr<engine_t>> engine_;
		#else // fallback for MSVC
		std::unique_ptr<engine_t> engine_;
		#endif

		num_solns_found_t num_solns_found_ {0};
	};
}


namespace okiidoku::visitor {
}
#endif