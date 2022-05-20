#include <okiidoku/puzzle/make.hpp>

#include <okiidoku/puzzle/solve.hpp>

#include <algorithm>
#include <numeric> // iota
#include <random> // TODO remove once we make a smarter puzzle maker. currently a dumb implementation.

namespace okiidoku::mono { namespace {
	//
}}
namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	void make_minimal_puzzle(Grid<O>& grid) noexcept {
		using T = Ints<O>;
		// using o2i_t = typename T::o2i_t;
		using rmi_t = typename T::o4x_smol_t;
		using o4i_t = typename T::o4i_t;

		using rng_t = std::minstd_rand;
		rng_t rng {std::random_device{}()};

		o4i_t num_puzcell_cands {T::O4};
		std::array<rmi_t, T::O4> puzcell_cand_rmis;
		// everything at or above the index `num_puzcell_cands` is either removed,
		// or would cause loss of properness if removed.
		std::iota(puzcell_cand_rmis.begin(), puzcell_cand_rmis.end(), 0);
		const auto remove_puzcell_cand_at {[&](const o4i_t cand_i){
			--num_puzcell_cands;
			std::swap(puzcell_cand_rmis[cand_i], puzcell_cand_rmis[num_puzcell_cands]);
		}};
		// hypothesis: if removing a given breaks properness, the only way to change that
		// is by adding back other removed givens. If this is true, once a given is known
		// to break properness

		while (num_puzcell_cands > 0) {
			const auto puzcell_cand_i {static_cast<o4i_t>((rng() - rng_t::min()) % num_puzcell_cands)};
			const auto rmi {puzcell_cand_rmis[puzcell_cand_i]};
			const auto val {grid.at_rmi(rmi)};
			if (val == T::O4) { continue; }
			grid.at_rmi(rmi) = T::O2;
			FastSolver solver {grid};
			if (const auto soln_optional_ {solver.get_next_solution()}; soln_optional_) {
				if (const auto another_soln {solver.get_next_solution()}; another_soln) {
					// multiple solutions now possible. removal would break properness. don't remove.
					grid.at_rmi(rmi) = val;
					remove_puzcell_cand_at(puzcell_cand_i); // uses above hypothesis.
				} else {
					// puzzle still proper (still only one solution possible). ok to remove.
					remove_puzcell_cand_at(puzcell_cand_i);
				}
			} else {
				// impossible. removing givens _never decreases_ the number of possible solutions.
				assert(false);
			}
		}
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template void make_minimal_puzzle<O_>(Grid<O_>&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}