#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

#include <okiidoku/house_mask.hpp>

#include <algorithm>
#include <array>
#include <vector>
#include <compare>

namespace okiidoku::mono::detail::solver { namespace {

	// opening boilerplate. #undef-ed before end of namespace.
	#define OKIIDOKU_CAND_ELIM_FINDER_PRELUDE \
		using T = Ints<O>; \
		using o2x_smol_t [[maybe_unused]] = typename T::o2x_smol_t; \
		using o2x_t [[maybe_unused]] = typename T::o2x_t; \
		using o2i_t = typename T::o2i_t; \
		using o3i_t [[maybe_unused]] = typename T::o3i_t; \
		using rmi_t [[maybe_unused]] = typename T::o4x_smol_t; \
		using o4i_t [[maybe_unused]] = typename T::o4i_t;


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

		// Note: from sudopedia: the hidden/naked subsets of size greater than
		//  (O2+1)//2 have complements of the other kind of smaller subset size.
		//  I am not using this property for any optimization because my finder
		//  for subsets searches for all subset-sizes in a single pass.
		// template<Order O> requires(is_order_compiled(O))
		// static constexpr unsigned max_subset_size {(Ints<O>::O2 + 1U) / 2U};

		template<Order O> requires(is_order_compiled(O))
		struct GroupMe final {
			HouseMask<O> cands;
			typename Ints<O>::o2x_smol_t who;
		};

		template<Order O> requires(is_order_compiled(O))
		using enqueue_subset_desc_fn_t = void (*)(
			CandElimQueues<O>& cand_elim_queues,
			const HouseMask<O>& what_required,
			const HouseMask<O>& who_requires,
			const typename Ints<O>::o2x_t house,
			const HouseType house_type
		);

		// TODO templating the whole helper just for the part that does emplace
		// may be wasteful in terms of generated code-size. Consider "erasing"
		// the template into the form of a callback parameter.
		template<Order O> requires(is_order_compiled(O))
		void helper_find(
			const typename EngineObj<O>::CandSymsGrid& cells_cands,
			CandElimQueues<O>& cand_elim_queues,
			const HouseType house_type,
			const typename Ints<O>::o2x_t house,
			std::array<GroupMe<O>, Ints<O>::O2>& searcher,
			enqueue_subset_desc_fn_t<O> enqueue_subset_desc_fn
		) noexcept {
			OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
			const auto group_me_cmp {[](const GroupMe<O>& a, const GroupMe<O>& b){
				// below line commented out because there is no point in first sorting by cand_count.
				// if (const auto cmp {a.cand_count <=> b.cand_count}; std::is_neq(cmp)) [[likely]] { return cmp; }
				return HouseMask<O>::unspecified_strong_cmp(a.cands, b.cands);
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
					if (alike_size == 1) { continue; } // we have a custom finder implementation for subset-size == 1.
					const auto& group {searcher[alike_cur-1]};
					if (alike_size == group.cands.count()) {
						HouseMask<O> who_requires {};
						for (o2i_t a_who {alike_begin}; a_who < alike_cur; ++a_who) {
							who_requires.set(static_cast<o2x_t>(a_who));
						}
						// TODO give the queue a config field for whether to insert new entries
						//  auto-sorted by subset size. currently unsorted. If we uncomment the
						//  above sort-by-cand-count in `group_me_cmp`, we may be able to leverage
						//  some kind of merge-sorted-arrays algorithm.
						enqueue_subset_desc_fn(cand_elim_queues, group.cands, who_requires, house, house_type);
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
		for (HouseType house_type : house_types) {
		for (o2i_t house {0}; house < T::O2; ++house) {
			std::array<subsets::GroupMe<O>, T::O2> searcher;
			for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
				const auto rmi {house_cell_to_rmi<O>(house_type, house, house_cell)};
				searcher[house_cell] = subsets::GroupMe<O>{
					.cands{cells_cands.at_rmi(rmi)},
					.who{static_cast<o2x_smol_t>(house_cell)}
				};
			}
			subsets::helper_find<O>(
				cells_cands, cand_elim_queues,
				house_type, static_cast<o2x_t>(house), searcher,
				[](auto& queues, const auto& what, const auto& who, const auto house, const auto house_type){
					queues.emplace(cand_elim_desc::CellsRequireSymbols<O>{
						what, who, house, house_type
					});
				}
			);
		}}
	}


	template<Order O> requires(is_order_compiled(O))
	void find_symbols_require_cells(
		const typename EngineObj<O>::CandSymsGrid& cells_cands,
		CandElimQueues<O>& cand_elim_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		for (HouseType house_type : house_types) {
		for (o2i_t house {0}; house < T::O2; ++house) {
			std::array<subsets::GroupMe<O>, T::O2> searcher {};
			// TODO.mid try applying loop-tiling to see if it improves cache-usage.
			for (o2i_t symbol {0}; symbol < T::O2; ++symbol) {
				searcher[symbol].who = static_cast<o2x_smol_t>(symbol);
				for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
					// Note: not a very cache-locality-friendly loop, but I can't
					// think of how to do better. Inverting the loop dimensions may be
					// worse, since GroupMe has the additional, unused-here .who member.
					const auto rmi {house_cell_to_rmi<O>(house_type, house, house_cell)};
					if (cells_cands.at_rmi(rmi).test(static_cast<o2x_t>(symbol))) {
						searcher[symbol].cands.set(static_cast<o2x_t>(house_cell));
					}
				}
			}
			subsets::helper_find<O>(
				cells_cands, cand_elim_queues,
				house_type, static_cast<o2x_t>(house), searcher,
				[](auto& queues, const auto& what, const auto& who, const auto house, const auto house_type){
					queues.emplace(cand_elim_desc::SymbolsRequireCells<O>{
						what, who, house, house_type
					});
				}
			);
		}}
	}


	template<Order O> requires(is_order_compiled(O))
	void find_locked_candidates(
		const typename EngineObj<O>::CandSymsGrid& cells_cands,
		CandElimQueues<O>& cand_elim_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		// TODO
	}


	template<Order O> requires(is_order_compiled(O))
	typename EngineObj<O>::Guess find_good_guess_candidate(
		const typename EngineObj<O>::CandSymsGrid& cells_cands,
		const typename Ints<O>::o4i_t num_puzzle_cells_remaining
	) noexcept{
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		using guess_t = typename EngineObj<O>::Guess;
		// some guiding intuition:
		// choose a guess which is likely to cascade into the most candidate
		// elimination deductions before the next required guess point. choose
		// a guess where finding out the guess is wrong will enable many
		// candidate elimination deductions.

		// start by finding cells with the fewest number of candidate-symbols.
		// Note: no combinations of std algorithms seems specialized enough for what I want.
		std::vector<rmi_t> cell_tags; {
			const o2i_t best_count {(num_puzzle_cells_remaining == T::O4) ? T::O2 : static_cast<o2i_t>(T::O2-1U)};
			for (o4i_t rmi {1}; rmi < T::O4; ++rmi) {
				const auto& cell_cands {cells_cands.at_rmi(rmi)};
				const auto cmp {cell_cands.count() <=> best_count};
				if (std::is_eq(cmp)) {
					cell_tags.push_back(static_cast<rmi_t>(rmi));
				} else if (std::is_lt(cmp) && cell_cands.count() > 1) {
					cell_tags.clear();
					cell_tags.push_back(static_cast<rmi_t>(rmi));
				}
			}
		}
		assert(cell_tags.size() > 0);
		return guess_t{
			.rmi{cell_tags[0]},
			.val{cells_cands.at_rmi(cell_tags[0]).count_lower_zeros_assuming_non_empty_mask()}
		};

		// then, for those cells, find the one whose candidate-symbols have
		// very few candidate-house-cells.
		// using house_cand_counts_t = std::array<o2i_t, house_types.size()>;
		// std::vector<house_cand_counts_t> cell_tag_sym_major_cand_count;
		// cell_tag_sym_major_cand_count.capacity(cell_tags.size());
		// for (const auto& tag_rmi : cell_tags) {
		// 	// TODO alternate design: instead of looping over the tag-cell's symbols
		// 	// to count, use (tag_cell_cands & nb_cell_cands).count().
		// 	for (const o2i_t sym : cells_cands.at_rmi(tag_rmi).set_bits_iter()) {
		// 		house_cand_counts_t house_cand_counts {{0}};
		// 		for (const auto& house_type : house_types) {
		// 		for (o2i_t nb_house_cell {0}; nb_house_cell < T::O2; ++nb_house_cell) {
		// 			const auto nb_rmi {house_cell_to_rmi<O>(
		// 				house_type,
		// 				rmi_to_house<O>(house_type, tag_rmi),
		// 				nb_house_cell
		// 			)}
		// 			if (cells_cands.at_rmi(nb_rmi).test(symbol)) {
		// 				++house_cand_counts[house_type];
		// 			}
		// 		}}
		// 		std::ranges::sort(house_cand_counts);
		// 	}
		// }
		// return guess_t{.rmi{rmi}, .val{val}};
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

	template<Order O> requires(is_order_compiled(O))
	typename EngineObj<O>::Guess CandElimFind<O>::good_guess_candidate(const EngineObj<O>& engine) noexcept {
		assert(!engine.no_solutions_remain());
		return find_good_guess_candidate<O>(engine.cells_cands_, engine.get_num_puzzle_cells_remaining());
	}


	#undef OKIIDOKU_CAND_ELIM_FINDER_PRELUDE

	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class CandElimFind<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}