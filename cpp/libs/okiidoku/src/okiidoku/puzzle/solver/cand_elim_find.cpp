#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

#include <okiidoku/house_mask.hpp>
#include <okiidoku/puzzle/solver/subset_combo_walker.hpp>

#include <range/v3/view/take.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/algorithm/fold.hpp>

#include <functional> // function, bit_or
#include <algorithm>
#include <array>
#include <vector>
#include <compare>

// TODO.low if this translation unit becomes slow to compile and specific functions
// are being frequently modified, consider experimenting with explicit instantiation
// at the granularity of member functions in separate translation units.

namespace okiidoku::mono::detail::solver { namespace {

	// opening boilerplate. #undef-ed before end of namespace.
	#define OKIIDOKU_CAND_ELIM_FINDER_PRELUDE \
		using T [[maybe_unused]] = Ints<O>; \
		using o2xs_t [[maybe_unused]] = int_ts::o2xs_t<O>; \
		using o2x_t [[maybe_unused]] = int_ts::o2x_t<O>; \
		using o2is_t [[maybe_unused]] = int_ts::o2is_t<O>; \
		using o2i_t [[maybe_unused]] = int_ts::o2i_t<O>; \
		using o3i_t [[maybe_unused]] = int_ts::o3i_t<O>; \
		using rmi_t [[maybe_unused]] = int_ts::o4xs_t<O>; \
		using o4i_t [[maybe_unused]] = int_ts::o4i_t<O>;


	template<Order O> requires(is_order_compiled(O))
	void find_sym_claim_cell(
		const CandsGrid<O>& cells_cands,
		CandElimQueues<O>& found_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		(void)cells_cands, (void)found_queues;
		// TODO for each house of all house-types, check if any symbol only has one candidate-house-cell.
		// how to use masks to optimize? have an accumulator candidate-symbol mask "<house-type>_seen_cand_syms" that starts as zeros.
		// have a <house-type>_syms_with_multiple_cand_cells accumulator that also starts as zeros.
		// for each cell in the house, OR= the <house-type>_syms_with_multiple_cand_cells with the AND of <house-type>_seen_cand_syms and the cell's candidate-symbols.
		//  for bits that have changed value, remove the corresponding entry from <house-type>_single_cand_house_cell_for_sym.
		// then OR= <house-type>_seen_cand_syms with the cell's candidate-symbols.
		//  for true bits that are not also true in <house-type>_syms_with_multiple_cand_cells, add the house-cell index to the corresponding entry of <house-type>_seen_cand_syms.

		// for any matches, _if the cell is not already committed (ie. if the cell still has more than one candidate-symbols), enqueue.
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
			int_ts::o2is_t<O> cand_count;
			int_ts::o2xs_t<O> who;
		};

		template<Order O> requires(is_order_compiled(O))
		using enqueue_subset_desc_fn_t = std::function<void(
			const HouseMask<O>& what_required,
			const HouseMask<O>& who_requires
		)>;

		template<Order O> requires(is_order_compiled(O))
		void helper_find(
			std::array<GroupMe<O>, Ints<O>::O2>& search_me,
			enqueue_subset_desc_fn_t<O> enqueue_subset_desc_fn
		) noexcept {
			OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
			const auto group_me_cmp {[](const GroupMe<O>& a, const GroupMe<O>& b){
				if (const auto cmp {a.cand_count <=> b.cand_count}; std::is_neq(cmp)) [[likely]] { return cmp; }
				return HouseMask<O>::cmp_differences(a.cands, b.cands); // TODO.try is this line beneficial? I actually don't even really know if it does what I intend, which is to put similar sets close together.
			}};
			std::sort(search_me.begin(), search_me.end(), [&](const auto& a, const auto& b){
				return std::is_lt(group_me_cmp(a,b));
			});
			for (o2x_t subset_size {2}; subset_size < T::O2; ++subset_size) {
				const auto search_begin {ranges::find_if(search_me, [](const auto& gm){
					return gm.cand_count > 1;
				})};
				const auto search_end {ranges::find_if(search_begin, search_me.end(), [&](const auto& gm){
					return gm.cand_count > subset_size;
				})};
				if (std::distance(search_begin, search_end) < subset_size) { continue; }
				// find all combinations of N sets where their union is size N.
				// given that the union's size will never be less than N, and that
				// any one set can only be part of one such finding.
				SubsetComboWalker<O> combo_walker {
					static_cast<o2x_t>(std::distance(search_me.begin(), search_begin)),
					static_cast<o2i_t>(std::distance(search_me.begin(), search_end)),
					subset_size,
				};
				for (; combo_walker.has_more(); combo_walker.advance()) {
					// walk combinations.
					// Note on "`*`". fold returns nullopt if range arg is empty and no init
					// arg is passed. That will never happen here so no need to check for nullopt.
					const auto combo_union {*ranges::fold_left_first(
						ranges::views::take(combo_walker.get_combo_arr(), subset_size)
						| ranges::views::transform([&](const auto i){ return search_me[i].cands; })
					, std::bit_or{})
					};
					if (combo_union.count()) [[unlikely]] {
						HouseMask<O> who_requires {};
						for (o2x_t i {0}; i < subset_size; ++i) {
							who_requires.set(search_me[combo_walker.combo_at(i)].who);
						}
						enqueue_subset_desc_fn(combo_union, who_requires);
					}
				}
			}
		}
	}


	// TODO consider adding an argument about whether to do candidate elimination eagerly or queue them.
	//  the annoying thing is that then the return type for the queue case doesn't need to be SolutionsRemain...
	//  in that case, we could make it a wrapper than just discards (void-casts) the result because it knows that it should never be SolutionsRemain(false) if queueing. can also assert that that is so.
	//  the main question: why would that be beneficial? in the eager case, I think there's the benefit of operating on masks that
	//  are already in cache. but is that a significant enough benefit? probably needs benchmark to justify...
	//  The eager function wrapper takes an argument for find_one vs find_all. the queueing wrapper returns void.
	template<Order O> requires(is_order_compiled(O))
	void find_cells_claim_syms(
		const CandsGrid<O>& cells_cands,
		CandElimQueues<O>& found_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		for (HouseType house_type : house_types) {
		for (o2i_t house {0}; house < T::O2; ++house) {
			std::array<subsets::GroupMe<O>, T::O2> search_me;
			for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
				const auto& cell_cands {cells_cands.at_rmi(house_cell_to_rmi<O>(house_type, house, house_cell))};
				search_me[house_cell] = subsets::GroupMe<O>{
					.cands{cell_cands},
					.cand_count{static_cast<o2is_t>(cell_cands.count())},
					.who{static_cast<o2xs_t>(house_cell)},
				};
			}
			subsets::helper_find<O>(search_me, [&](const auto& what, const auto& who){
				found_queues.emplace(found::CellsClaimSyms<O>{
					what, who, static_cast<o2xs_t>(house), house_type
				});
			});
		}}
	}


	template<Order O> requires(is_order_compiled(O))
	void find_syms_claim_cells(
		const CandsGrid<O>& cells_cands,
		CandElimQueues<O>& found_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		for (HouseType house_type : house_types) {
		for (o2i_t house {0}; house < T::O2; ++house) {
			std::array<subsets::GroupMe<O>, T::O2> search_me {};
			// TODO.mid try applying loop-tiling to see if it improves cache-usage.
			for (o2i_t symbol {0}; symbol < T::O2; ++symbol) {
				auto& group_me {search_me[symbol]};
				group_me.who = static_cast<o2xs_t>(symbol);
				for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
					const auto rmi {house_cell_to_rmi<O>(house_type, house, house_cell)};
					if (cells_cands.at_rmi(rmi).test(static_cast<o2x_t>(symbol))) {
						group_me.cands.set(static_cast<o2x_t>(house_cell));
					}
				}
				group_me.cand_count = static_cast<o2is_t>(group_me.cands.count());
			}
			subsets::helper_find<O>(search_me, [&](const auto& what, const auto& who){
				found_queues.emplace(found::SymsClaimCells<O>{
					what, who, static_cast<o2xs_t>(house), house_type
				});
			});
		}}
	}


	template<Order O> requires(is_order_compiled(O))
	void find_locked_cands(
		const CandsGrid<O>& cells_cands,
		CandElimQueues<O>& found_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		(void)cells_cands, (void)found_queues;// TODO
	}


	template<Order O> requires(is_order_compiled(O))
	Guess<O> find_good_guess_candidate(
		const CandsGrid<O>& cells_cands,
		const int_ts::o4i_t<O> num_puzcells_remaining
	) noexcept{
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		// some guiding intuition:
		// choose a guess which is likely to cascade into the most candidate
		// elimination deductions before the next required guess point. choose
		// a guess where finding out the guess is wrong will enable many
		// candidate elimination deductions.

		// start by finding cells with the fewest number of candidate-symbols.
		// Note: no combinations of std algorithms seems specialized enough for what I want.
		std::vector<rmi_t> cell_tags; {
			const o2i_t best_count {(num_puzcells_remaining == T::O4) ? T::O2 : static_cast<o2i_t>(T::O2-1U)};
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
		assert(cell_tags.empty());
		return Guess<O>{
			.rmi{cell_tags[0]},
			.val{cells_cands.at_rmi(cell_tags[0]).count_lower_zeros_assuming_non_empty_mask()},
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
		// return Guess<O>{.rmi{rmi}, .val{val}};
	}


	#undef OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
}}


namespace okiidoku::mono::detail::solver {

	// opening boilerplate. #undef-ed before end of namespace.
	#define OKIIDOKU_CAND_ELIM_FINDER_PRELUDE \
		assert(!engine.no_solutions_remain()); \
		if (engine.get_num_puzcells_remaining() == 0) [[unlikely]] { return; }


	template<Order O> requires(is_order_compiled(O))
	void CandElimFind<O>::sym_claim_cell(Engine<O>& engine) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		return find_sym_claim_cell(engine.cells_cands(), engine.found_queues());
	}

	template<Order O> requires(is_order_compiled(O))
	void CandElimFind<O>::cells_claim_syms(Engine<O>& engine) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		return find_cells_claim_syms(engine.cells_cands(), engine.found_queues());
	}

	template<Order O> requires(is_order_compiled(O))
	void CandElimFind<O>::syms_claim_cells(Engine<O>& engine) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		return find_syms_claim_cells(engine.cells_cands(), engine.found_queues());
	}

	template<Order O> requires(is_order_compiled(O))
	void CandElimFind<O>::locked_cands(Engine<O>& engine) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_PRELUDE
		return find_locked_cands(engine.cells_cands(), engine.found_queues());
	}

	template<Order O> requires(is_order_compiled(O))
	Guess<O> CandElimFind<O>::good_guess_candidate(const Engine<O>& engine) noexcept {
		assert(!engine.no_solutions_remain());
		return find_good_guess_candidate<O>(engine.cells_cands(), engine.get_num_puzcells_remaining());
	}


	#undef OKIIDOKU_CAND_ELIM_FINDER_PRELUDE

	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class CandElimFind<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}