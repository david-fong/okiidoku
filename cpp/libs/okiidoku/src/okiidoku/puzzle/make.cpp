// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/puzzle/make.hpp>

#include <okiidoku/puzzle/solve.hpp>
#include <okiidoku/puzzle/ua_set.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <algorithm>
#include <execution>
#include <random> // TODO remove once we make a smarter puzzle maker. currently a dumb implementation.
#include <array>
#include <utility>

#include <iostream>

namespace okiidoku::mono { namespace {
}}
namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	bool grid_is_proper_puzzle(const Grid<O>& puzzle) noexcept {
		if (grid_is_filled(puzzle)) [[unlikely]] {
			return grid_follows_rule(puzzle);
		}
		// a quick check for obvious big mistake (empty grid):
		if (grid_is_empty(puzzle)) [[unlikely]] { return false; }

		FastSolver<O> solver {};
		solver.reinit_with_puzzle(puzzle);
		return solver.get_next_solution().has_value() && !solver.get_next_solution().has_value();
	}


	template<Order O> requires(is_order_compiled(O))
	bool grid_is_minimal_puzzle(const Grid<O>& grid) noexcept {
		Grid<O> copy {grid};
		make_minimal_puzzle(copy, rng_seed_t{0u});
		return grid == copy;
	}


	template<Order O> requires(is_order_compiled(O))
	void make_minimal_puzzle(Grid<O>& grid, const rng_seed_t rng_seed) noexcept {
		OKIIDOKU_CONTRACT_ASSERT(grid_is_proper_puzzle(grid));

		const auto ua_sets {find_size_4_minimal_unavoidable_sets(grid)};

		using rng_t = std::minstd_rand;
		rng_t rng {rng_seed};

		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		using rmi_t = T::o4xs_t;
		// Note: this implementation never "backtracks". once it removes a given,
		// it never puts it back.

		o4i_t num_puzcell_cands {0u};
		OKIIDOKU_DEFER_INIT // NOLINTNEXTLINE(*-init)
		std::array<rmi_t, T::O4> puzcell_cand_rmis; // non-candidates: either removed, or can't be removed.
		for (const auto rmi : T::O4) {
			if (grid[rmi] < T::O2) [[likely]] {
				puzcell_cand_rmis[num_puzcell_cands] = rmi;
				++num_puzcell_cands;
		}	}

		const auto remove_puzcell_cand_at {[&](const o4i_t cand_i){
			OKIIDOKU_CONTRACT_USE(cand_i < num_puzcell_cands);
			--num_puzcell_cands;
			std::swap(puzcell_cand_rmis[cand_i], puzcell_cand_rmis[num_puzcell_cands]);
		}};
		o4i_t num_keepers {0u};

		// Note: will remove the logging once I'm done working on solver (ie. far in the future)
		static constexpr auto call_debug_log_fn {[]([[maybe_unused]] auto&& fn){
			if constexpr (O < 5) { // NOLINT(*-branch-clone, *magic-number*)
				#ifndef NDEBUG
				fn();
				#endif
			} else {
				fn();
			}
		}};

		FastSolver<O> solver {};
		while (num_puzcell_cands > 0u) {
			const auto puzcell_cand_i {(rng()-rng_t::min()) % num_puzcell_cands};
			const auto rmi {puzcell_cand_rmis[puzcell_cand_i]};
			const auto sym {std::exchange(grid[rmi], T::O2)};

			call_debug_log_fn([&]{
				std::clog << "\n\n#puzcell cands: " << int(num_puzcell_cands) << ". try rm @ " << int(rmi) << std::flush;
			});
			// if (num_puzcell_cands <= 61) [[unlikely]] { return; } // TODO delete when done profiling

			// if (std::any_of(
			// 	ua_sets.ua_set_4s.cbegin(),
			// 	ua_sets.ua_set_4s.cend(),
			// 	[&](const auto& ua_set_4){
			// 		return std::count_if(
			// 			OKIIDOKU_UNSEQ
			// 			ua_set_4.rmis.cbegin(),
			// 			ua_set_4.rmis.cend(),
			// 			[&](const auto& ua_set_rmi){ return (ua_set_rmi != rmi) && (grid[ua_set_rmi] != T::O2); }
			// 		) == 0;
			// 	}
			// )) {
			// 	std::clog << "\ncannot remove at " << int(rmi) << " since it is the last given for a ua_set_4.";
			// }

			solver.reinit_with_puzzle(grid, {{.rmi{rmi}, .sym{*sym}}});
			if (const auto new_soln_opt {solver.get_next_solution()}; new_soln_opt) {
				// multiple solutions now possible. removal would break properness. don't remove.
				call_debug_log_fn([&]{
					std::clog << "\nmultiple solutions possible! rm failed" << std::flush;
				});
				grid[rmi] = sym;
				++num_keepers;
			}
			remove_puzcell_cand_at(puzcell_cand_i);
			// OKIIDOKU_CONTRACT_ASSERT(grid_follows_rule(grid)); // a bit gratuitous
		}
		OKIIDOKU_CONTRACT_ASSERT(grid_is_proper_puzzle(grid));
	}


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
	template bool grid_is_proper_puzzle<(O_)>(const Grid<(O_)>&) noexcept; \
	template bool grid_is_minimal_puzzle<(O_)>(const Grid<(O_)>&) noexcept; \
	template void make_minimal_puzzle<(O_)>(Grid<(O_)>&, rng_seed_t) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}


namespace okiidoku::visitor {

	bool grid_is_proper_puzzle(const Grid& grid) noexcept {
		switch (grid.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return mono::grid_is_proper_puzzle<(O_)>(grid.unchecked_get_mono_exact<(O_)>());
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	bool grid_is_minimal_puzzle(const Grid& grid) noexcept {
		switch (grid.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return mono::grid_is_minimal_puzzle<(O_)>(grid.unchecked_get_mono_exact<(O_)>());
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	void make_minimal_puzzle(Grid& grid, const rng_seed_t rng_seed) noexcept {
		switch (grid.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return mono::make_minimal_puzzle<(O_)>(grid.unchecked_get_mono_exact<(O_)>(), rng_seed);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}
}