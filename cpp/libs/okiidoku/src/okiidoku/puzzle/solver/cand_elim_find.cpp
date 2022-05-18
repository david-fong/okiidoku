#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

#include <okiidoku/house_mask.hpp>

#include <algorithm>
#include <compare>

namespace okiidoku::mono::detail::solver { namespace {

	// opening boilerplate. #undef-ed before end of namespace.
	#define OKIIDOKU_CAND_ELIM_FINDER_PRELUDE \
		using T = Ints<O>; \
		using o2i_t = typename T::o2i_t; \
		using val_t = typename T::o2x_smol_t; \
		using rmi_t = typename T::o4x_smol_t; \
		using o4i_t = typename T::o4i_t; \
		using cand_syms_t = typename EngineObj<O>::cand_syms_t;


	template<Order O> requires(is_order_compiled(O))
	void find_symbol_requires_cell(
		const typename EngineObj<O>::CandSymsGrid& cells_cands,
		CandElimQueues<O>& cand_elim_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
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


	namespace subsets {

		template<Order O> requires(is_order_compiled(O))
		static constexpr unsigned max_subset_size {};

		template<Order O> requires(is_order_compiled(O))
		struct GroupMe final {
			HouseMask<O> cands;
			typename Ints<O>::o2i_smol_t cand_count;
			typename Ints<O>::o2x_smol_t who;
		};

		template<Order O, class DescT> requires(is_order_compiled(O)
			&& (std::is_same_v<DescT, cand_elim_desc::CellsRequireSymbols<O>>
			 || std::is_same_v<DescT, cand_elim_desc::SymbolsRequireCells<O>>
			)
		)
		void helper_find(
			const typename EngineObj<O>::CandSymsGrid& cells_cands,
			CandElimQueues<O>& cand_elim_queues,
			const HouseType house_type,
			const typename Ints<O>::o2x_t house,
			std::array<GroupMe<O>, Ints<O>::O2>& searcher
		) noexcept {
			OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
			using o3i_t = typename T::o3i_t;
			const auto group_me_cmp {[&](const GroupMe<O>& a, const GroupMe<O>& b){
				// TODO.asap wait... is there really a point in first sorting by cand_count? I... don;t think so...
				if (const auto cmp {a.cand_count <=> b.cand_count}; std::is_neq(cmp)) [[likely]] {
					return cmp;
				}
				return HouseMask<O>::unspecified_strong_cmp(
					cells_cands.at_rmi(house_cell_to_rmi<O>(house_type, house, a.who)),
					cells_cands.at_rmi(house_cell_to_rmi<O>(house_type, house, b.who))
				);
			}};
			std::sort(searcher.begin(), searcher.end(), [&](const GroupMe<O>& a, const GroupMe<O>& b){
				return std::is_lt(group_me_cmp(a,b));
			});
			{o2i_t alike_begin {0};
			for (o3i_t alike_cur {1}; alike_cur <= T::O2; ++alike_cur) {
				assert(alike_cur > alike_begin);
				if ((alike_cur == T::O2)
				|| std::is_neq(group_me_cmp(searcher[alike_cur-1], searcher[alike_cur]))
				) {
					const auto alike_size {static_cast<o2i_t>(alike_cur - alike_begin)};
					assert(alike_size > 0);
					if (alike_size == 1) { continue; } // /we have custom implementation for subsets of size one.
					const auto& group {searcher[alike_cur-1]};
					if (alike_size == group.cand_count) {
						HouseMask<O> who_requires {};
						for (o2i_t a_who {alike_begin}; a_who < alike_cur; ++a_who) {
							who_requires.set(a_who);
						}
						cand_elim_queues.emplace(DescT{group.cands, who_requires, house, house_type});
					}
					alike_begin = static_cast<decltype(alike_begin)>(alike_cur);
				}
			}}
		}
	}


	// TODO consider adding an argument about whether to do candidate elimination eagerly or queue them.
	//  the annoying thing is that then the return type for the queue case doesn't need to be SolutionsRemain...
	//  in that case, we could make it a wrapper than just discards (void-casts) the result because it knows that it should never be SolutionsRemain(false) if queueing. can also assert that that is so.
	//  the main question: why would that be beneficial? in the eager case, I think theres the benefit of operating on masks that
	//  are already in cache. but is that a significant enough benefit? probably needs benchmark to justify...
	//  The eager function wrapper takes an argument for find_one vs find_all. the queueing wrapper returns void.
	template<Order O> requires(is_order_compiled(O))
	void find_cells_require_symbols(
		const typename EngineObj<O>::CandSymsGrid& cells_cands,
		CandElimQueues<O>& cand_elim_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		// TODO
		for (HouseType house_type : HouseTypes) {
		for (o2i_t house {0}; house < T::O2; ++house) {
			std::array<subsets::GroupMe<O>, T::O2> searcher;
			for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
				const auto rmi {house_cell_to_rmi<O>(house_type, house, house_cell)};
				const auto& cands {cells_cands.at_rmi(rmi)};
				searcher[house_cell] = subsets::GroupMe<O>{
					.cands{cands},
					.cand_count{static_cast<typename T::o2i_smol_t>(cands.count())},
					.who{static_cast<typename T::o2x_smol_t>(house_cell)}
				};
			}
			subsets::helper_find<O, cand_elim_desc::CellsRequireSymbols<O>>(
				cells_cands, cand_elim_queues, house_type, house, searcher
			);
		}}
		const o2i_t house {0};
	}


	template<Order O> requires(is_order_compiled(O))
	void find_symbols_require_cells(
		const typename EngineObj<O>::CandSymsGrid& cells_cands,
		CandElimQueues<O>& cand_elim_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		// for each house type, for each house of that type, for each symbol
		// get a mask for each symbol of which cells it can be in in that house.
		// then apply the same technique.
		// TODO
	}


	template<Order O> requires(is_order_compiled(O))
	void find_locked_candidates(
		const typename EngineObj<O>::CandSymsGrid& cells_cands,
		CandElimQueues<O>& cand_elim_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		// TODO
	}


	#undef OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
}}


namespace okiidoku::mono::detail::solver {

	// opening boilerplate. #undef-ed before end of namespace.
	#define OKIIDOKU_CAND_ELIM_FINDER_PRELUDE \
		assert(!engine.no_solutions_remain()); \
		if (engine.get_num_puzzle_cells_remaining() == 0) [[unlikely]] { return; }


	template<Order O> requires(is_order_compiled(O))
	void CandElimFind<O>::symbol_requires_cell(EngineObj<O>& engine) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		return find_symbol_requires_cell(engine.cells_cands_, engine.cand_elim_queues_);
	}

	template<Order O> requires(is_order_compiled(O))
	void CandElimFind<O>::cells_require_symbols(EngineObj<O>& engine) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		return find_cells_require_symbols(engine.cells_cands_, engine.cand_elim_queues_);
	}

	template<Order O> requires(is_order_compiled(O))
	void CandElimFind<O>::symbols_require_cells(EngineObj<O>& engine) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		return find_symbols_require_cells(engine.cells_cands_, engine.cand_elim_queues_);
	}

	template<Order O> requires(is_order_compiled(O))
	void CandElimFind<O>::locked_candidates(EngineObj<O>& engine) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		return find_locked_candidates(engine.cells_cands_, engine.cand_elim_queues_);
	}


	#undef OKIIDOKU_CAND_ELIM_FINDER_PRELUDE

	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class CandElimFind<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}