#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

#include <okiidoku/puzzle/solver/found.hpp>

#include <algorithm>
#include <execution>
#include <array>
#include <vector>

#include <okiidoku/puzzle/solver/cand_elim_find.macros.hpp>

namespace okiidoku::mono::detail::solver { namespace {

	/*
	hypothesis: making guesses all packed in the same / nearby line-box intersections
	increases the probability of finding "contradictory" guesses earlier, since those
	guesses will concentrate their eliminations in the same lines and boxes.

	question: could there be a correlation between good guess candidates and cells that
	could not be removed as givens ("keepers")? Try making a guess-suggester that favours
	guessing at a cell which sees many keepers.
	*/

	template<Order O> requires(is_order_compiled(O))
	Guess<O> find_good_guess_candidate(
		const CandsGrid<O>& cells_cands
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		// some guiding intuition:
		// choose a guess which is likely to cascade into the most candidate
		// elimination deductions before the next required guess point. choose
		// a guess where finding out the guess is wrong will enable many
		// candidate elimination deductions.

		using house_solved_counts_t = std::array<o2is_t, house_types.size()>;
		std::array<house_solved_counts_t, T::O2> houses_solved_counts {};
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			const auto cand_count {cells_cands.at_rmi(rmi).count()};
			for (const auto house_type : house_types) {
				if (cand_count == 1) [[unlikely]] {
					++(houses_solved_counts[rmi_to_house<O>(house_type, rmi)][static_cast<unsigned char>(house_type)]);
		}	}	}
		const auto get_house_solved_counts {[&](const o4i_t rmi){
			house_solved_counts_t _ {0};
			for (const auto house_type : house_types) {
				const auto ht {static_cast<unsigned char>(house_type)};
				const auto house {rmi_to_house<O>(house_type, rmi)};
				_[ht] = houses_solved_counts[house][ht];
			}
			std::sort(_.begin(), _.end(), std::less{});
			return _;
		}};
		o4i_t best_rmi {T::O4};
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			if (cells_cands.at_rmi(rmi).count() > 1) [[likely]] {
				best_rmi = rmi;
				break;
			}
		}
		assert(best_rmi < T::O4);
		o2i_t best_count {cells_cands.at_rmi(best_rmi).count()};
		auto best_house_solved_counts {get_house_solved_counts(best_rmi)};

		for (o4i_t rmi {static_cast<o4i_t>(best_rmi+1U)}; rmi < T::O4; ++rmi) {
			const auto cand_count {cells_cands.at_rmi(rmi).count()};
			if (cand_count <= 1) [[unlikely]] { continue; } // no guessing for solved cell.
			if (cand_count > best_count) [[likely]] { continue; }
			auto house_solved_counts {get_house_solved_counts(rmi)};
			if (cand_count < best_count || (house_solved_counts < best_house_solved_counts)) [[unlikely]] {
				best_rmi = rmi;
				best_count = cand_count;
				best_house_solved_counts = house_solved_counts;
			}
		}
		return Guess<O>{
			.rmi{static_cast<rmi_t>(best_rmi)},
			.val{cells_cands.at_rmi(best_rmi).count_lower_zeros_assuming_non_empty_mask()},
		};
	}


	#undef OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
}}


namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	Guess<O> CandElimFind<O>::good_guess_candidate(const Engine<O>& engine) noexcept {
		assert(!engine.no_solutions_remain());
		assert(engine.get_num_puzcells_remaining() > 0); // why guess when solved?
		return find_good_guess_candidate<O>(engine.cells_cands());
	}

	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template Guess<O_> CandElimFind<O_>::good_guess_candidate(const Engine<O_>&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}