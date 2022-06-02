#include <okiidoku/puzzle/make.hpp>

#include <okiidoku/puzzle/solve.hpp>

#include <algorithm>
#include <numeric> // iota
#include <random> // TODO remove once we make a smarter puzzle maker. currently a dumb implementation.

#include <iostream>

namespace okiidoku::mono { namespace {

	template<Order O> requires(is_order_compiled(O))
	Grid<O> get_og_soln_of_proper_puzzle_for_debug(const Grid<O>& proper_puzzle) noexcept {
		Grid<O> og_soln;
		if (grid_is_filled(proper_puzzle)) {
			og_soln = proper_puzzle;
		} else {
			FastSolver solver {proper_puzzle};
			if (const auto soln1_opt {solver.get_next_solution()}; soln1_opt) {
				og_soln = soln1_opt.value();
				if (const auto soln2_opt {solver.get_next_solution()}; soln2_opt) {
					assert(false); // contract: input puzzle is proper
				}
			} else {
				assert(false); // contract: input puzzle is proper
			}
		}
		return og_soln;
	}
}}
namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	void make_minimal_puzzle(Grid<O>& grid, const rng_seed_t rng_seed) noexcept {
		assert(grid_follows_rule(grid));

		#ifndef NDEBUG
		// setup for assertion that input and output have the same solution.
		Grid<O> og_soln {get_og_soln_of_proper_puzzle_for_debug(grid)};
		#endif

		using rng_t = std::minstd_rand;
		rng_t rng {rng_seed};

		using T = Ints<O>;
		// using o2i_t = int_ts::o2i_t<O>;
		using rmi_t = int_ts::o4xs_t<O>;
		using o4i_t = int_ts::o4i_t<O>;

		o4i_t num_puzcell_cands {0};
		std::array<rmi_t, T::O4> puzcell_cand_rmis; // non-candidates: either removed, or can't be removed.
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			if (grid.at_rmi(rmi) < T::O2) [[likely]] {
				puzcell_cand_rmis[num_puzcell_cands] = static_cast<rmi_t>(rmi);
				++num_puzcell_cands;
		}	}
		assert(num_puzcell_cands > 0); // partially enforces contract: input is proper puzzle.

		const auto remove_puzcell_cand_at {[&](const o4i_t cand_i){
			assert(cand_i < num_puzcell_cands);
			--num_puzcell_cands;
			puzcell_cand_rmis[cand_i] = std::move(puzcell_cand_rmis[num_puzcell_cands]);
		}};

		while (num_puzcell_cands > 0) {
			const auto puzcell_cand_i {static_cast<o4i_t>((rng() - rng_t::min()) % num_puzcell_cands)};
			assert(puzcell_cand_i < num_puzcell_cands);
			const auto rmi {puzcell_cand_rmis[puzcell_cand_i]};
			const auto val {std::exchange(grid.at_rmi(rmi), T::O2)};
			assert(val < T::O2);
			std::clog << "\n\n" << int(num_puzcell_cands) << " puzcell candidates remain. attempting remove at rmi=" << int(rmi) << std::flush;
			FastSolver solver {grid};
			if (const auto soln1_opt {solver.get_next_solution()}; soln1_opt) {
				if (const auto soln2_opt {solver.get_next_solution()}; soln2_opt) {
					// multiple solutions now possible. removal would break properness. don't remove.
					std::clog << "\nmultiple solutions possible! rm failed" << std::flush; // TODO delete this
					grid.at_rmi(rmi) = val;
				} else {
					// puzzle still proper (still only one solution possible). ok to remove.
					std::clog << "\nrm success" << std::flush; // TODO delete this
					assert(soln1_opt.value() == og_soln);
				}
				remove_puzcell_cand_at(puzcell_cand_i);
				assert(grid_follows_rule(grid));
			} else {
				// impossible. removing givens _never decreases_ the number of possible solutions.
				// also contract that input is a proper puzzle.
				assert(false);
			}
		}
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template void make_minimal_puzzle<O_>(Grid<O_>&, rng_seed_t) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}