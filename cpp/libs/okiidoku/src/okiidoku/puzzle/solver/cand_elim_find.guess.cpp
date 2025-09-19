// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

#include <okiidoku/puzzle/solver/found.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <numeric>    // transform_reduce <- will not be needed if we do the get_guess_grouping bookkeeping optimization
#include <algorithm>  // sort
#include <functional> // greater
#include <execution>
#include <array>
#include <tuple>      // tie (for comparisons)

#include <okiidoku/puzzle/solver/cand_elim_find.macros.hpp>

namespace okiidoku::mono::detail::solver { namespace {

	/*
	question: could there be a correlation between good guess candidates and cells that
	could not be removed as givens ("keepers")? Try making a guess-suggester that favours
	guessing at a cell which sees many keepers.
	*/

	template<Order O> requires(is_order_compiled(O))
	Ints<O>::o2xs_t find_good_guess_sym_for_cell(
		const CandsGrid<O>& cells_cands,
		const typename Ints<O>::o4x_t rmi
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		const auto best_cell_cands {cells_cands.at_rmi(rmi)};
		const auto get_sym_num_other_cand_cells {[&](const o2x_t sym){
			o3i_t num_other_cand_cells {0};
			for (const auto house_type : house_types) {
				const auto house {rmi_to_house<O>(house_type, rmi)};
				for (const auto house_cell : T::O2) {
					const auto& other_cell {cells_cands.at_rmi(house_cell_to_rmi<O>(house_type, house, house_cell))};
					if (other_cell[sym]) { ++num_other_cand_cells; }
				}
			}
			return num_other_cand_cells;
		}};
		auto best_sym {best_cell_cands.first_set_bit_require_exists()};
		o3i_t best_sym_num_other_cand_cells {0};
		for (const auto sym : best_cell_cands.set_bits()) {
			const auto sym_num_other_cand_cells {get_sym_num_other_cand_cells(sym)};
			if (sym_num_other_cand_cells > best_sym_num_other_cand_cells) [[unlikely]] {
				best_sym = sym;
				best_sym_num_other_cand_cells = sym_num_other_cand_cells;
			}
		}
		return best_sym;
	}


	/** \pre a guess candidate exists. */
	template<Order O> requires(is_order_compiled(O))
	// NOLINTNEXTLINE(*-cognitive-complexity)
	Guess<O> find_good_guess_candidate_for_fast_solver(
		const CandsGrid<O>& cells_cands,
		[[maybe_unused]] const typename EngineImpl<O>::guess_stack_t& guess_stack
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS

		using house_solved_counts_t = HouseTypeMap<o2is_t>;
		// TODO consider a way of caching this information in the engine data? I can't guess whether it will be better or worse. This isn't like the guess network, I think bookkeeping for this may require one copy of the info for each stack frame to make sense from a POV of avoiding re-computation.
		std::array<house_solved_counts_t, T::O2> houses_solved_counts {};
		for (const auto rmi : T::O4) {
			if (cells_cands.at_rmi(rmi).count() == 1u) [[unlikely]] {
				for (const auto house_type : house_types) {
					++(houses_solved_counts[rmi_to_house<O>(house_type, rmi)].at(house_type));
		}	}	}
		const auto get_house_solved_counts {[&](const o4x_t rmi){
			if constexpr (O < 5) { // NOLINT(readability-magic-numbers)
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
		// TODO.try doing bookkeeping in the engine to avoid re-computation. (keep a guess-count for each house). slightly more relevant now that large fields of guess stack frames aren't heap-allocated (ie. the guess field of each stack entry are farther apart).
		[[maybe_unused]] const auto get_guess_grouping {[&](const o4x_t rmi) -> std::uintmax_t {
			return std::transform_reduce(
				#ifdef __cpp_lib_execution
				std::execution::unseq,
				#endif
				guess_stack.cbegin(), guess_stack.cend(), std::uintmax_t{0u}, std::plus<std::uintmax_t>{},
				[rmi](const auto& frame) -> auto {
					const auto other_rmi {frame.guess.rmi};
					return (// TODO consider using gcc's __builtin_expect to annotate as unlikely. standard attribute cannot be used for ternary.
					  (rmi_to_row<O>(rmi) == rmi_to_row<O>(other_rmi) ? 1u : 0u) // consider extracting this into a `count_shared_houses` function that returns Int<3>
					+ (rmi_to_col<O>(rmi) == rmi_to_col<O>(other_rmi) ? 1u : 0u)
					+ (rmi_to_box<O>(rmi) == rmi_to_box<O>(other_rmi) ? 1u : 0u));
				}
			);
		}};
		o4x_t best_rmi {[&](){
			for (const auto rmi : T::O4) {
				if (cells_cands.at_rmi(rmi).count() > 1u) [[likely]] {
					return rmi;
			}	}
			OKIIDOKU_UNREACHABLE;
		}()};
		auto best_cand_count {cells_cands.at_rmi(best_rmi).count()};
		auto best_house_solved_counts {get_house_solved_counts(best_rmi)};
		[[maybe_unused]] auto best_guess_grouping {get_guess_grouping(best_rmi)};

		// TODO is there a same-or-better-perf way to write this search using std::transform_reduce or std::min?
		for (o4i_t rmi {best_rmi.next()}; rmi < T::O4; ++rmi) {
			const auto cand_count {cells_cands.at_rmi(*rmi).count()};
			OKIIDOKU_CONTRACT_USE(cand_count != 0u);
			if (cand_count <= 1u) [[unlikely]] { continue; } // no guessing for solved cell.
			[[maybe_unused]] const auto guess_grouping {get_guess_grouping(*rmi)};
			[[maybe_unused]] const auto house_solved_counts {get_house_solved_counts(*rmi)};
			if ([&]{
				if constexpr (O <= 3u) {
					return std::tie(     cand_count)
			  		     < std::tie(best_cand_count);
				} else if constexpr (O <= 4u) {
					return std::tie(     cand_count,      house_solved_counts)
			  		     < std::tie(best_cand_count, best_house_solved_counts);
				} else {
					return std::tie(     cand_count,      guess_grouping, best_house_solved_counts)
			  		     < std::tie(best_cand_count, best_guess_grouping,      house_solved_counts);
				}
			}()) [[unlikely]] {
				best_rmi = *rmi;
				best_cand_count = cand_count;
				best_house_solved_counts = house_solved_counts;
				best_guess_grouping = guess_grouping;
			}
		}
		return Guess<O>{
			.rmi{static_cast<rmi_t>(best_rmi)},
			.val{find_good_guess_sym_for_cell(cells_cands, best_rmi)},
		};
	}


	#undef OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
}}


namespace okiidoku::mono::detail::solver {

	template<Order O> requires(is_order_compiled(O))
	Guess<O> CandElimFind<O>::good_guess_candidate(const Engine<O>& engine) noexcept {
		OKIIDOKU_CONTRACT_ASSERT(!engine.no_more_solns());
		OKIIDOKU_CONTRACT_ASSERT(engine.get_num_unsolved() > 0); // cannot guess when already solved
		return find_good_guess_candidate_for_fast_solver<O>(engine.cells_cands(), engine.get_guess_stack_());
	}

	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template Guess<O_> CandElimFind<O_>::good_guess_candidate(const Engine<O_>&) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}