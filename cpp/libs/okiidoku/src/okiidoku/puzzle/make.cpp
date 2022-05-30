#include <okiidoku/puzzle/make.hpp>

#include <okiidoku/puzzle/solve.hpp>

#include <algorithm>
#include <numeric> // iota
#include <random> // TODO remove once we make a smarter puzzle maker. currently a dumb implementation.

#include <iostream>

namespace okiidoku::mono { namespace {
	//
}}
namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	void make_minimal_puzzle(Grid<O>& grid, const rng_seed_t rng_seed) noexcept {
		assert(grid_follows_rule(grid));

		#ifndef NDEBUG
		// setup for assertion that input and output have the same solution.
		Grid<O> og_soln;
		if (grid_is_filled(grid)) {
			og_soln = grid;
		} else {
			FastSolver solver {grid};
			if (const auto soln1_opt {solver.get_next_solution()}; soln1_opt) {
				og_soln = soln1_opt.value();
				if (const auto soln2_opt {solver.get_next_solution()}; soln2_opt) {
					assert(false); // contract: input puzzle is proper
				}
			} else {
				assert(false); // contract: input puzzle is proper
			}
		}
		#endif

		using rng_t = std::minstd_rand;
		rng_t rng {rng_seed};

		using T = Ints<O>;
		// using o2i_t = int_ts::o2i_t<O>;
		using rmi_t = int_ts::o4xs_t<O>;
		using o4i_t = int_ts::o4i_t<O>;

		o4i_t num_puzcell_cands {0};
		std::array<rmi_t, T::O4> puzcell_cand_rmis;
		// everything at or above the index `num_puzcell_cands` is either removed,
		// or would cause loss of properness if removed.
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			if (grid.at_rmi(rmi) < T::O2) [[likely]] {
				puzcell_cand_rmis[num_puzcell_cands] = static_cast<rmi_t>(rmi);
				++num_puzcell_cands;
			}
		}
		assert(num_puzcell_cands > 0); // partially enforces contract: input is proper puzzle.

		const auto remove_puzcell_cand_at {[&](const o4i_t cand_i){
			assert(cand_i < num_puzcell_cands);
			--num_puzcell_cands;
			puzcell_cand_rmis[cand_i] = std::move(puzcell_cand_rmis[num_puzcell_cands]);
		}};
		// hypothesis: if removing a given breaks properness, the only way to change that
		// is by adding back other removed givens. If this is true, once a given is known
		// to break properness, if no backtracking is done, then it can never be removed.

		while (num_puzcell_cands > 0) {
			const auto puzcell_cand_i {static_cast<o4i_t>((rng() - rng_t::min()) % num_puzcell_cands)};
			assert(puzcell_cand_i < num_puzcell_cands);
			const auto rmi {puzcell_cand_rmis[puzcell_cand_i]};
			const auto val {std::exchange(grid.at_rmi(rmi), T::O2)};
			assert(val < T::O2);
			FastSolver solver {grid};
			std::clog << "\n\n" << int(num_puzcell_cands) << ' ';
			if (const auto soln1_opt {solver.get_next_solution()}; soln1_opt) {
				if (const auto soln2_opt {solver.get_next_solution()}; soln2_opt) {
					// multiple solutions now possible. removal would break properness. don't remove.
					grid.at_rmi(rmi) = val;
					std::clog << "\nrm failed " << int(rmi) << std::flush; // TODO delete this
				} else {
					// puzzle still proper (still only one solution possible). ok to remove.
					assert(soln1_opt.value() == og_soln);
					std::clog << "\nrm " << int(rmi) << std::flush; // TODO delete this
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