#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

#include <okiidoku/puzzle/solver/found.hpp>

#include <algorithm>
#include <array>
#include <vector>

#include <okiidoku/puzzle/solver/cand_elim_find.macros.hpp>

namespace okiidoku::mono::detail::solver { namespace {


	template<Order O> requires(is_order_compiled(O))
	[[nodiscard]] bool find_locked_cands_and_check_needs_unwind(
		const CandsGrid<O>& cells_cands,
		FoundQueues<O>& found_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		(void)cells_cands, (void)found_queues;// TODO
		return false;
	}


	template<Order O> requires(is_order_compiled(O))
	Guess<O> find_good_guess_candidate(
		const CandsGrid<O>& cells_cands,
		const int_ts::o4i_t<O> num_puzcells_remaining
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		assert(num_puzcells_remaining > 1);
		// some guiding intuition:
		// choose a guess which is likely to cascade into the most candidate
		// elimination deductions before the next required guess point. choose
		// a guess where finding out the guess is wrong will enable many
		// candidate elimination deductions.

		// start by finding cells with the fewest number of candidate-symbols.
		// Note: no combinations of std algorithms seems specialized enough for what I want.
		std::vector<rmi_t> cand_rmis; {
			o2i_t best_count {(num_puzcells_remaining == T::O4) ? T::O2 : static_cast<o2i_t>(T::O2-1U)};
			for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
				const auto cand_count {cells_cands.at_rmi(rmi).count()};
				if (cand_count == best_count) {
					cand_rmis.push_back(static_cast<rmi_t>(rmi));
				} else if (cand_count < best_count && cand_count > 1U) {
					best_count = cand_count;
					cand_rmis.clear();
					cand_rmis.push_back(static_cast<rmi_t>(rmi));
				}
			}
		}
		assert(!cand_rmis.empty());
		assert(std::all_of(cand_rmis.cbegin(), cand_rmis.cend(), [&](const auto& rmi){
			const auto& cell_cands {cells_cands.at_rmi(rmi)};
			return cell_cands.count() > 1;
		}));
		return Guess<O>{
			.rmi{cand_rmis[0]},
			.val{cells_cands.at_rmi(cand_rmis[0]).count_lower_zeros_assuming_non_empty_mask()},
		};

		// then, for those cells, find the one whose candidate-symbols have
		// very few candidate-house-cells.
		// using house_cand_counts_t = std::array<o2i_t, house_types.size()>;
		// std::vector<house_cand_counts_t> cell_tag_sym_major_cand_count;
		// cell_tag_sym_major_cand_count.capacity(cand_rmis.size());
		// for (const auto& tag_rmi : cand_rmis) {
		// 	// TODO alternate design: instead of looping over the tag-cell's symbols
		// 	// to count, use (tag_cell_cands & nb_cell_cands).count().
		// 	for (const o2i_t sym : cells_cands.at_rmi(tag_rmi).set_bits_iter()) {
		// 		house_cand_counts_t house_cand_counts {{0}};
		// 		for (auto house_type : house_types) {
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


	#undef OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
}}


namespace okiidoku::mono::detail::solver {

	OKIIDOKU_CAND_ELIM_FINDER_DEF(locked_cands)
	#undef OKIIDOKU_CAND_ELIM_FINDER_DEF


	template<Order O> requires(is_order_compiled(O))
	Guess<O> CandElimFind<O>::good_guess_candidate(const Engine<O>& engine) noexcept {
		assert(!engine.no_solutions_remain());
		return find_good_guess_candidate<O>(engine.cells_cands(), engine.get_num_puzcells_remaining());
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template UnwindInfo CandElimFind<O_>::locked_cands(Engine<O_>&) noexcept; \
		template Guess<O_> CandElimFind<O_>::good_guess_candidate(const Engine<O_>&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}