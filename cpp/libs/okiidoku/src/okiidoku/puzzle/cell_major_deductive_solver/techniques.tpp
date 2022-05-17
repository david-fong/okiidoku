#ifndef TPP_OKIIDOKU__PUZZLE__CELL_MAJOR_DEDUCTIVE_SOLVER__TECHNIQUES
#define TPP_OKIIDOKU__PUZZLE__CELL_MAJOR_DEDUCTIVE_SOLVER__TECHNIQUES

#include <okiidoku/puzzle/cell_major_deductive_solver/engine.hpp>
#include <okiidoku/puzzle/cand_elim_desc.hpp>

#include <algorithm>
#include <compare>

namespace okiidoku::mono::detail::cell_major_deductive_solver {

	namespace techniques::subsets {
		template<Order O> requires(is_order_compiled(O))
		static constexpr unsigned max_subset_size {};
	}

	template<Order O> requires(is_order_compiled(O))
	class Techniques final {
		using T = Ints<O>;
		using o2i_t = typename T::o2i_t;
		using val_t = typename T::o2x_smol_t;
		using rmi_t = typename T::o4x_smol_t;
		using o4i_t = typename T::o4i_t;
		using cand_syms_t = typename EngineObj<O>::cand_syms_t;

		// Internal (somewhat obvious) contract:
		// techniques must never incorrectly progress in solving a proper puzzle.

	public:
		// common contracts and invariants for all techniques:
		// contract: `no_solutions_remain` returns `false`.
		// behaviour: immediately returns if `get_num_puzzle_cells_remaining` returns zero.

		// TODO.high  for techniques that are not (relatively) trivial to perform a full scan of the grid,
		//   provide a parameter enum `SearchEffort { find_first, find_all, };`
		static void find_symbol_requires_cell(EngineObj<O>&) noexcept;

		static void find_locked_candidates(EngineObj<O>&) noexcept;

		// AKA "naked subsets"
		// // contract: `subset_size` is in the range [2, ((O2+1)//2)].
		static void find_cells_requiring_symbols(EngineObj<O>&/* val_t subset_size */) noexcept;

		// AKA "hidden subsets"
		// // contract: `subset_size` is in the range [2, ((O2+1)//2)].
		static void find_symbols_requiring_cells(EngineObj<O>&/* val_t subset_size */) noexcept;

		static void find_fish(EngineObj<O>&) noexcept;
	};


	// opening boilerplate. #undef-ed before end of namespace.
	#define OKIIDOKU_TECHNIQUE_PRELUDE \
		assert(!e.no_solutions_remain()); \
		if (e.get_num_puzzle_cells_remaining() == 0) [[unlikely]] { return; }


	template<Order O> requires(is_order_compiled(O))
	void Techniques<O>::find_symbol_requires_cell(EngineObj<O>& e) noexcept {
		OKIIDOKU_TECHNIQUE_PRELUDE
		// TODO for each house of all house-types, check if any symbol only has one candidate-house-cell.
		// how to use masks to optimize? have an accumulator candidate-symbol mask "<house-type>_seen_cand_syms" that starts as zeros.
		// have a <house-type>_syms_with_multiple_cand_cells accumulator that also starts as zeros.
		// for each cell in the house, OR= the <house-type>_syms_with_multiple_cand_cells with the AND of <house-type>_seen_cand_syms and the cell's candidate-symbols.
		//  for bits that have changed value, remove the corresponding entry from <house-type>_single_cand_house_cell_for_sym.
		// then OR= <house-type>_seen_cand_syms with the cell's candidate-symbols.
		//  for true bits that are not also true in <house-type>_syms_with_multiple_cand_cells, add the house-cell index to the corresponding entry of <house-type>_seen_cand_syms.

		// for any matches, _if the cell is not already committed (ie. if the cell still has more than one candidate-symbols),
		//  call register_new_given_();
	}


	// TODO consider adding an argument about whether to do candidate elimination eagerly or queue them.
	//  the annoying thing is that then the return type for the queue case doesn't need to be SolutionsRemain...
	//  in that case, we could make it a wrapper than just discards (void-casts) the result because it knows that it should never be unsat if queueing. can also assert that it is not unsat.
	//  the main question: why would that be beneficial? in the eager case, I think theres the benefit of operating on masks that
	//  are already in cache. but is that a significant enough benefit? probably needs benchmark to justify...
	//  The eager function wrapper takes an argument for find_one vs find_all. the queueing wrapper returns void.
	template<Order O> requires(is_order_compiled(O))
	void Techniques<O>::find_cells_requiring_symbols(EngineObj<O>& e
		// const EngineObj<O>::val_t subset_size
	) noexcept {
		// assert(subset_size > 1);
		// assert(subset_size <= techniques::subsets::max_subset_size<O>);
		OKIIDOKU_TECHNIQUE_PRELUDE
		using cand_count_t = typename T::o2i_smol_t;
		struct GroupMe final {
			cand_count_t cand_count;
			rmi_t house_cell_i;
		};
		// TODO
		// for each house type, for each house,
		const o2i_t row {0};
		std::array<GroupMe, T::O2> subset_searcher;
		for (o2i_t i {0}; i < T::O2; ++i) {
			const auto rmi {static_cast<rmi_t>((T::O2*row)+i)};
			subset_searcher[i] = GroupMe{
				.cand_count{static_cast<cand_count_t>(e.cells_cands_.at_rmi(rmi).count())},
				.house_cell_i{rmi}
			};
		}
		const auto group_me_cmp {[&](const GroupMe& a, const GroupMe& b){
			if (const auto cmp {a.cand_count <=> b.cand_count}; std::is_neq(cmp)) [[likely]] {
				return cmp;
			}
			return cand_syms_t::unspecified_strong_cmp(
				e.cells_cands_.at(row, a.house_cell_i),
				e.cells_cands_.at(row, b.house_cell_i)
			);
		}};
		std::sort(subset_searcher.begin(), subset_searcher.end(), [&](const GroupMe& a, const GroupMe& b){
			return std::is_lt(group_me_cmp(a,b));
		});
		{
			o2i_t alike_begin {0};
			for (o2i_t alike_cur {1}; alike_cur < T::O2; ++alike_cur) {
				assert(alike_cur > alike_begin);
				if (std::is_neq(group_me_cmp(subset_searcher[alike_cur-1], subset_searcher[alike_cur]))) {
					const auto alike_size {static_cast<o2i_t>(alike_cur - alike_begin)};
					assert(alike_size > 0);
					if (alike_size == 1) { continue; }
					if (alike_size == subset_searcher[alike_cur-1].cand_count) {
						// TODO either process or enqueue.
					}
					alike_begin = alike_cur;
				}
			}
		}
	}


	// TODO I think there should be a way to do a bit of data prep and then basically reuse the code of cells_requiring_symbols
	template<Order O> requires(is_order_compiled(O))
	void Techniques<O>::find_symbols_requiring_cells(EngineObj<O>& e
		// const EngineObj<O>::val_t subset_size
	) noexcept {
		OKIIDOKU_TECHNIQUE_PRELUDE
		// for each house type, for each house of that type, for each symbol
		// get a mask for each symbol of which cells it can be in in that house.
		// then apply the same technique.
		// TODO
	}


	template<Order O> requires(is_order_compiled(O))
	void Techniques<O>::find_locked_candidates(EngineObj<O>& e) noexcept {
		OKIIDOKU_TECHNIQUE_PRELUDE
		// TODO
	}


	#undef OKIIDOKU_TECHNIQUE_PRELUDE


	// disable implicit template instantiation.
	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		extern template class Techniques<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}
#endif