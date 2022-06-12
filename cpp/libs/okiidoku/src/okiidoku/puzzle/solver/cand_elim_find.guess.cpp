#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

#include <okiidoku/puzzle/solver/found.hpp>

#include <numeric> // transform_reduce
#include <algorithm> // sort
#include <execution>
#include <ranges>
#include <array>
#include <tuple> // tie (for comparisons)

#include <okiidoku/puzzle/solver/cand_elim_find.macros.hpp>

namespace okiidoku::mono::detail::solver { namespace {

	/*
	question: could there be a correlation between good guess candidates and cells that
	could not be removed as givens ("keepers")? Try making a guess-suggester that favours
	guessing at a cell which sees many keepers.
	*/

	template<Order O> requires(is_order_compiled(O))
	Guess<O> find_good_guess_candidate_for_fast_solver(
		const CandsGrid<O>& cells_cands,
		[[maybe_unused]] const typename EngineImpl<O>::guess_stack_t& guess_stack
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS

		using house_solved_counts_t = HouseTypeMap<o2is_t>;
		// TODO consider a way of caching this information? I can't guess whether it will be better or worse. This isn't like the guess network, I think bookkeeping for this may require one copy of the info for each stack frame to make sense from a POV of avoiding re-computation.
		std::array<house_solved_counts_t, T::O2> houses_solved_counts {};
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			if (cells_cands.at_rmi(rmi).count() == 1) [[unlikely]] {
				for (const auto house_type : house_types) {
					++(houses_solved_counts[rmi_to_house<O>(house_type, rmi)].at(house_type));
		}	}	}
		const auto get_house_solved_counts {[&](const o4i_t rmi){
			if constexpr (O < 5) {
				o3i_t _ {0};
				for (const auto house_type : house_types) {
					const auto house {rmi_to_house<O>(house_type, rmi)};
					_ += houses_solved_counts[house].at(house_type);
				}
				return _;
			} else {
				house_solved_counts_t _;
				for (const auto house_type : house_types) {
					const auto house {rmi_to_house<O>(house_type, rmi)};
					_.at(house_type) = houses_solved_counts[house].at(house_type);
				}
				std::sort(_.get_underlying_arr().begin(), _.get_underlying_arr().end(), std::greater{});
				return _.get_underlying_arr();
			}
		}};
		// TODO.try doing bookkeeping in the engine to avoid re-computation. (keep a guess-count for each house)
		[[maybe_unused]] const auto get_guess_grouping {[&](const o4i_t rmi) -> o3i_t {
			return std::transform_reduce(
				#ifdef __cpp_lib_execution
				std::execution::unseq,
				#endif
				guess_stack.cbegin(), guess_stack.cend(), static_cast<o3i_t>(0), std::plus<o3i_t>{},
				[rmi](const auto& frame) -> o3i_t {
					const auto other_rmi {frame.guess.rmi};
					return // TODO consider using gcc's __builtin_expect to annotate as unlikely. standard attribute cannot be used for ternary.
					  (rmi_to_row<O>(rmi) == rmi_to_row<O>(other_rmi) ? 1 : 0)
					+ (rmi_to_col<O>(rmi) == rmi_to_col<O>(other_rmi) ? 1 : 0)
					+ (rmi_to_box<O>(rmi) == rmi_to_box<O>(other_rmi) ? 1 : 0);
				}
			);
		}};
		o4i_t best_rmi {T::O4};
		for (o4i_t rmi {0}; rmi < T::O4; ++rmi) {
			if (cells_cands.at_rmi(rmi).count() > 1) [[likely]] {
				best_rmi = rmi;
				break;
		}	}
		OKIIDOKU_CONTRACT_TRIVIAL_EVAL(best_rmi < T::O4);
		auto best_cand_count {cells_cands.at_rmi(best_rmi).count()};
		auto best_house_solved_counts {get_house_solved_counts(best_rmi)};
		[[maybe_unused]] auto best_guess_grouping {get_guess_grouping(best_rmi)};

		// TODO is there a same-or-better-perf way to write this search using std::min?
		for (o4i_t rmi {static_cast<o4i_t>(best_rmi+1U)}; rmi < T::O4; ++rmi) {
			const auto cand_count {cells_cands.at_rmi(rmi).count()};
			if (cand_count <= 1) [[unlikely]] { continue; } // no guessing for solved cell.
			[[maybe_unused]] const auto guess_grouping {get_guess_grouping(rmi)};
			[[maybe_unused]] const auto house_solved_counts {get_house_solved_counts(rmi)};
			if ([&]{
				if constexpr (O <= 3) {
					return std::tie(     cand_count)
			  		     < std::tie(best_cand_count);
				} else if constexpr (O <= 4) {
					return std::tie(     cand_count,      house_solved_counts)
			  		     < std::tie(best_cand_count, best_house_solved_counts);
				} else {
					return std::tie(     cand_count,      guess_grouping, best_house_solved_counts)
			  		     < std::tie(best_cand_count, best_guess_grouping,      house_solved_counts);
				}
			}()) [[unlikely]] {
				best_rmi = rmi;
				best_cand_count = cand_count;
				best_house_solved_counts = house_solved_counts;
				best_guess_grouping = guess_grouping;
			}
		}
		return Guess<O>{
			.rmi{static_cast<rmi_t>(best_rmi)},
			.val{cells_cands.at_rmi(best_rmi).count_lower_zeros_assuming_non_empty_mask()},
			// TODO search for better way to choose which sym to guess.
		};
	}


	#undef OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
}}


namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	Guess<O> CandElimFind<O>::good_guess_candidate(const Engine<O>& engine) noexcept {
		assert(!engine.no_solutions_remain());
		assert(engine.get_num_puzcells_remaining() > 0); // cannot guess when already solved
		return find_good_guess_candidate_for_fast_solver<O>(engine.cells_cands(), engine.get_guess_stack_());
	}

	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template Guess<O_> CandElimFind<O_>::good_guess_candidate(const Engine<O_>&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}