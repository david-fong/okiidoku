#include <okiidoku/puzzle/make.hpp>

#include <okiidoku/puzzle/solve.hpp>

#include <algorithm>
#include <numeric> // iota
#include <random> // TODO remove once we make a smarter puzzle maker. currently a dumb implementation.

#include <iostream>

namespace okiidoku::mono { namespace {

	template<Order O> requires(is_order_compiled(O))
	[[nodiscard]] bool puzzle_is_proper(const Grid<O>& proper_puzzle) noexcept {
		if (grid_is_filled(proper_puzzle)) {
			return grid_follows_rule(proper_puzzle);
		}
		// a quick check for obvious big mistake (empty grid):
		if (grid_is_empty(proper_puzzle)) { return false; }

		FastSolver<O> solver {};
		solver.reinit_with_puzzle(proper_puzzle);
		return solver.get_next_solution().has_value() && !solver.get_next_solution().has_value();
	}
}}
namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	void make_minimal_puzzle(Grid<O>& grid, const rng_seed_t rng_seed) noexcept {
		assert(puzzle_is_proper(grid));

		using rng_t = std::minstd_rand;
		rng_t rng {static_cast<rng_t::result_type>(rng_seed)};

		using T = Ints<O>;
		using o2x_t = int_ts::o2x_t<O>;
		using rmi_t = int_ts::o4xs_t<O>;
		using o4i_t = int_ts::o4i_t<O>;
		// Note: this implementation never "backtracks". once it removes a given,
		// it never puts it back.

		o4i_t num_puzcell_cands {0};
		std::array<rmi_t, T::O4> puzcell_cand_rmis; // non-candidates: either removed, or can't be removed.
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			if (grid.at_rmi(rmi) < T::O2) [[likely]] {
				puzcell_cand_rmis[num_puzcell_cands] = static_cast<rmi_t>(rmi);
				++num_puzcell_cands;
		}	}

		const auto remove_puzcell_cand_at {[&](const o4i_t cand_i){
			assert(cand_i < num_puzcell_cands);
			--num_puzcell_cands;
			puzcell_cand_rmis[cand_i] = std::move(puzcell_cand_rmis[num_puzcell_cands]);
		}};
		o4i_t num_keepers {0};

		FastSolver<O> solver {};
		while (num_puzcell_cands > 0) {
			const auto puzcell_cand_i {static_cast<o4i_t>((rng() - rng_t::min()) % num_puzcell_cands)};
			assert(puzcell_cand_i < num_puzcell_cands);

			const auto rmi {puzcell_cand_rmis[puzcell_cand_i]};
			const auto val {std::exchange(grid.at_rmi(rmi), T::O2)};
			assert(val < T::O2);

			#ifndef NDEBUG
			std::clog << "\n\n#puzcell cands: " << int(num_puzcell_cands) << ". try rm @ " << int(rmi) << std::flush;
			#endif
			solver.reinit_with_puzzle(grid, {{.rmi{rmi}, .val{static_cast<o2x_t>(val)}}});

			if (const auto new_soln_opt {solver.get_next_solution()}; new_soln_opt) {
				// multiple solutions now possible. removal would break properness. don't remove.
				#ifndef NDEBUG
				std::clog << "\nmultiple solutions possible! rm failed" << std::flush;
				#endif
				grid.at_rmi(rmi) = val;
				++num_keepers;
			}
			remove_puzcell_cand_at(puzcell_cand_i);
			// assert(grid_follows_rule(grid)); // a bit gratuitous
		}
		assert(puzzle_is_proper(grid));
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template void make_minimal_puzzle<O_>(Grid<O_>&, rng_seed_t) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}