#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

#include <okiidoku/puzzle/solver/found.hpp>

#include <numeric> // transform_reduce
#include <algorithm> // sort
#include <execution>
#include <array>
#include <tuple> // tie (for comparisons)

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
		const CandsGrid<O>& cells_cands,
		[[maybe_unused]] const typename EngineImpl<O>::guess_stack_t& guess_stack
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
			if constexpr (O < 4) {
				o3i_t _ {0};
				for (const auto house_type : house_types) {
					const auto ht {static_cast<unsigned char>(house_type)};
					const auto house {rmi_to_house<O>(house_type, rmi)};
					_ += houses_solved_counts[house][ht];
				}
				return _;
			} else {
				house_solved_counts_t _;
				for (const auto house_type : house_types) {
					const auto ht {static_cast<unsigned char>(house_type)};
					const auto house {rmi_to_house<O>(house_type, rmi)};
					_[ht] = houses_solved_counts[house][ht];
				}
				std::sort(_.begin(), _.end(), std::greater{});
				return _;
			}
		}};
		[[maybe_unused]] const auto get_network_size {[&](const o4i_t rmi) -> o3i_t {
			return std::transform_reduce(
				#ifdef __cpp_lib_execution
				std::execution::unseq,
				#endif
				guess_stack.cbegin(), guess_stack.cend(), static_cast<o3i_t>(0), std::plus<o3i_t>{},
				[rmi](const auto& frame) -> o3i_t {
					o3i_t count {0};
					const auto other_rmi {frame.guess.rmi};
					if (rmi_to_row<O>(rmi) == rmi_to_row<O>(other_rmi)) [[unlikely]] { ++count; }
					if (rmi_to_col<O>(rmi) == rmi_to_col<O>(other_rmi)) [[unlikely]] { ++count; }
					if (rmi_to_box<O>(rmi) == rmi_to_box<O>(other_rmi)) [[unlikely]] { ++count; }
					return count;
				}
			);
		}};
		o4i_t best_rmi {T::O4};
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			if (cells_cands.at_rmi(rmi).count() > 1) [[likely]] {
				best_rmi = rmi;
				break;
		}	}
		assert(best_rmi < T::O4);
		auto best_cand_count {cells_cands.at_rmi(best_rmi).count()}; // lower is better
		auto best_house_solved_counts {get_house_solved_counts(best_rmi)}; // lex smaller is better (?)
		[[maybe_unused]] auto best_network_size {get_network_size(best_rmi)}; // larger is better (I think?)

		for (o4i_t rmi {static_cast<o4i_t>(best_rmi+1U)}; rmi < T::O4; ++rmi) {
			const auto cand_count {cells_cands.at_rmi(rmi).count()};
			if (cand_count <= 1) [[unlikely]] { continue; } // no guessing for solved cell.

			// consider alternatively writing as saving cmp vars for each comparison
			// so can "early exit" as soon as one is not better (before calculating
			// ingredients for future comparisons).
			const auto network_size {get_network_size(rmi)}; // TODO this doesn't seem to help :/
			const auto house_solved_counts {get_house_solved_counts(rmi)};
			if (std::tie(/* best_network_size,  */     cand_count,      house_solved_counts)
			  < std::tie(/*      network_size,  */best_cand_count, best_house_solved_counts)
			) [[unlikely]] {
				best_rmi = rmi;
				best_cand_count = cand_count;
				best_house_solved_counts = house_solved_counts;
				best_network_size = network_size;
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
		assert(engine.get_num_puzcells_remaining() > 0); // cannot guess when already solved
		return find_good_guess_candidate<O>(engine.cells_cands(), engine.get_guess_stack_());
	}

	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template Guess<O_> CandElimFind<O_>::good_guess_candidate(const Engine<O_>&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}