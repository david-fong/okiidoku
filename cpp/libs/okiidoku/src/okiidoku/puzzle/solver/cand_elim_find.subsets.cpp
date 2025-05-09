// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

#include <okiidoku/puzzle/solver/subset_combo_walker.hpp>
#include <okiidoku/puzzle/solver/found.hpp>
#include <okiidoku/o2_bit_arr.hpp>

#include <functional> // bit_or
#include <numeric> // transform_reduce
#include <algorithm> // sort
#include <execution>
#include <array>
#include <optional>

#include <okiidoku/puzzle/solver/cand_elim_find.macros.hpp>

namespace okiidoku::mono::detail::solver { namespace {

	template<Order O> requires(is_order_compiled(O))
	[[nodiscard]] bool find_sym_claim_cell_and_check_needs_unwind(
		const CandsGrid<O>& cells_cands,
		FoundQueues<O>& found_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		for (const auto house_type : house_types) {
		for (o2i_t house {0}; house < T::O2; ++house) {
			O2BitArr<O> syms_claiming_a_cell {O2BitArr_ones<O>}; {
				O2BitArr<O> syms_seen {};
				for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
					const auto& cell_cands {cells_cands.at_rmi(house_cell_to_rmi<O>(house_type, house, house_cell))};
					syms_claiming_a_cell.remove(syms_seen & cell_cands);
					syms_seen |= cell_cands;
				}
				if (syms_seen.count() < T::O2) [[unlikely]] {
					return true; // sym(s) with no cand cells.
				}
			}
			for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
				const auto rmi {house_cell_to_rmi<O>(house_type, house, house_cell)};
				const auto& cell_cands {cells_cands.at_rmi(rmi)};
				const auto match_cands {cell_cands & syms_claiming_a_cell};
				if (match_cands.count() > 0) [[unlikely]] {
					if (match_cands.count() > 1) [[unlikely]] { return true; } // multiple syms want same cell.
					const auto sym {match_cands.first_set_bit_require_exists()};
					if (cell_cands.count() > 1) [[likely]] {
						found_queues.push_back(found::SymClaimCell<O>{
							.rmi{static_cast<rmi_t>(rmi)},
							.val{static_cast<o2xs_t>(sym)},
						});
						// TODO consider flagging house for re-scan?
				}	}
			}
		}}
		return false; // no unwind needed.
	}


	template<Order O> requires(is_order_compiled(O))
	[[nodiscard]] bool prepare_try_decompose_subset_and_check_can_skip(
		const CandsGrid<O>& cells_cands,
		typename EngineImpl<O>::HouseSubsets& subs,
		int_ts::o2i_t<O>& sub_a,
		const int_ts::o2i_t<O> sub_z
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		OKIIDOKU_CONTRACT_USE(sub_z <= T::O2);
		OKIIDOKU_CONTRACT_USE(sub_a < sub_z);
		// update candidate-symbol count cache fields for the subset:
		{
			// TODO profile and add likelihood attributes
			bool no_change {true};
			for (o2i_t i {sub_a}; i < sub_z; ++i) {
				auto& cell_tag {subs.cell_tags[i]};
				const auto updated_count {static_cast<o2is_t>(cells_cands.at_rmi(cell_tag.rmi).count())};
				OKIIDOKU_CONTRACT_USE(cell_tag.count_cache <= T::O2);
				OKIIDOKU_CONTRACT_USE(cell_tag.count_cache >= updated_count);
				if (cell_tag.count_cache > updated_count) {
					no_change = false;
					cell_tag.count_cache = updated_count;
				}
			}
			if (no_change) { // TODO I think this is wrong: hidden subsets could still be found if cells outside this subset have changed: in particular, by losing a cand-sym that is a cand-sym of a cell inside this subset.
				sub_a = sub_z;
				return true;
			}
		}
		// sort to enable the `sub_sized_end` optimization and size-one-subset updates:
		std::sort(
			std::next(subs.cell_tags.begin(), static_cast<long>(sub_a)),
			std::next(subs.cell_tags.begin(), static_cast<long>(sub_z)),
			[&](const auto& tag_a, const auto& tag_b){
				return tag_a.count_cache < tag_b.count_cache;
			}
		);
		// detect and update state for already-found `CellClaimSym`:
		if (subs.cell_tags[sub_a].count_cache == 1) [[unlikely]] {
			do {
				++sub_a;
				if (sub_a == T::O2) [[unlikely]] { break; }
				subs.is_begin.set(static_cast<o2x_t>(sub_a));
			} while (sub_a < sub_z && subs.cell_tags[sub_a].count_cache == 1);
			return true; // TODO somehow optimize to skip the above sort when re-loop?
		}
		return false;
	}


	enum class NakedOrHidden { naked, hidden };

	template<Order O> requires(is_order_compiled(O))
	struct FoundSubsetInfo final {
		SubsetComboWalker<O> combo_walker;
		O2BitArr<O> combo_syms;
		NakedOrHidden naked_or_hidden;
	};


	template<Order O> requires(is_order_compiled(O))
	std::optional<FoundSubsetInfo<O>> try_decompose_subset(
		const CandsGrid<O>& cells_cands,
		const typename EngineImpl<O>::HouseSubsets& subs,
		const int_ts::o2i_t<O> sub_a,
		const int_ts::o2i_t<O> sub_z,
		const int_ts::o2x_t<O> max_subset_size // try up-to-and-including this size
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		OKIIDOKU_CONTRACT_USE(sub_z <= T::O2);
		OKIIDOKU_CONTRACT_USE(sub_a < sub_z);
		OKIIDOKU_CONTRACT_USE(max_subset_size >= 2);
		OKIIDOKU_CONTRACT_USE(max_subset_size <= static_cast<o2x_t>((T::O2+1)/2));
		for (o2x_t subset_i {0}; subset_i < (sub_z-sub_a-3) && subset_i+2 < 2*max_subset_size; ++subset_i) {
			const auto naked_or_hidden {(subset_i % 2 == 0) ? NakedOrHidden::naked : NakedOrHidden::hidden};
			const auto naked_subset_size {[&]() -> o2x_t {
				if (naked_or_hidden == NakedOrHidden::naked) {
					return static_cast<o2x_t>(2+(subset_i/2));
				}
				return static_cast<o2x_t>((sub_z-sub_a)-2-(subset_i/2));
			}()};
			OKIIDOKU_CONTRACT_USE(sub_a+naked_subset_size+1 < sub_z);
			// ^plus one to skip finding hidden singles. // TODO or also try to find them?
			std::optional<FoundSubsetInfo<O>> found {{ // Note: wrap with optional to allow NRVO
				.combo_walker {
					static_cast<o2x_t>(sub_a),
					[&]{
						auto sized_z {sub_a};
						while (sized_z < T::O2 && subs.cell_tags[sized_z].count_cache <= naked_subset_size) { ++sized_z; }
						return sized_z;
					}(),
					naked_subset_size,
				},
				.combo_syms {},
				.naked_or_hidden {naked_or_hidden},
			}};
			auto& [combo_walker, combo_syms, ignore_] {*found};
			for (; combo_walker.has_more(); combo_walker.advance()) {
				combo_syms = std::transform_reduce(
					#ifdef __cpp_lib_execution
					std::execution::unseq,
					#endif
					combo_walker.at_it(),
					std::next(combo_walker.at_it(), naked_subset_size),
					O2BitArr<O>{}, std::bit_or{}, [&](const auto i) -> const auto& {
						return cells_cands.at_rmi(subs.cell_tags[i].rmi);
					}
				);
				if (combo_syms.count() <= naked_subset_size) [[unlikely]] {
					return found;
				}
			}
		}
		return std::nullopt;
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo find_subsets_for_house_and_check_needs_unwind(
		EngineImpl<O>& engine,
		typename EngineImpl<O>::HouseSubsets& subs,
		const int_ts::o2x_t<O> max_subset_size // try up-to-and-including this size
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		OKIIDOKU_CONTRACT_USE(max_subset_size >= 2);
		OKIIDOKU_CONTRACT_USE(max_subset_size <= static_cast<o2x_t>((T::O2+1)/2));
		o2i_t sub_a {0};
		o2i_t sub_z {0};
		const auto get_next_sub_a {[&]{
			OKIIDOKU_CONTRACT_USE(sub_a < T::O2);
			auto next {static_cast<o2i_t>(sub_a+1)};
			while (next < T::O2 && !subs.is_begin.test(static_cast<o2x_t>(next))) { ++next; }
			OKIIDOKU_CONTRACT_USE(next <= T::O2);
			OKIIDOKU_CONTRACT_USE(next > sub_a);
			return next;
		}};
		// optional speed optimization to skip leading already-found singles:
		if (subs.is_begin.count() == T::O2) [[unlikely]] {
			return UnwindInfo::make_no_unwind();
		} else {
			auto non_first_members {O2BitArr_ones<O>};
			non_first_members.remove(subs.is_begin);
			const auto second_non_single_member {non_first_members.first_set_bit_require_exists()};
			OKIIDOKU_CONTRACT_USE(second_non_single_member > 0);
			sub_a = static_cast<o2i_t>(second_non_single_member-1);
		}

		while (sub_a < T::O2) [[likely]] {
			sub_z = get_next_sub_a();
			OKIIDOKU_CONTRACT_USE(sub_z <= T::O2);
			OKIIDOKU_CONTRACT_USE(sub_a < sub_z);
			OKIIDOKU_CONTRACT_ASSERT(subs.is_begin.test(static_cast<o2x_t>(sub_a)));
			for (auto i {static_cast<o2i_t>(sub_a+1)}; i < sub_z; ++i) {
				OKIIDOKU_CONTRACT_ASSERT(!subs.is_begin.test(static_cast<o2x_t>(i)));
			}
			if (prepare_try_decompose_subset_and_check_can_skip(engine.cells_cands(), subs, sub_a, sub_z)) {
				continue;
			}
			const auto found {try_decompose_subset(
				engine.cells_cands(), subs, sub_a, sub_z, max_subset_size
			)};
			if (!found.has_value()) [[likely]] {
				sub_a = sub_z;
				continue;
			}
			const auto& [combo_walker, combo_syms, naked_or_hidden] {*found};
			const auto naked_subset_size {combo_walker.get_naked_subset_size()};
			OKIIDOKU_CONTRACT_USE(combo_syms.count() <= naked_subset_size);
			if (combo_syms.count() < naked_subset_size) [[unlikely]] {
				return unwind_one_stack_frame_of_(engine);
			}
			if (naked_or_hidden == NakedOrHidden::naked) {
				for (o2x_t combo_at {0}; combo_at < naked_subset_size; ++combo_at) {
					auto& entry {subs.cell_tags[combo_walker.combo_at(combo_at)]};
					std::swap(subs.cell_tags[sub_a], entry);
					++sub_a;
				}
				subs.is_begin.set(static_cast<o2x_t>(sub_a));
			} else {
				for (o2x_t combo_at {0}; combo_at < naked_subset_size; ++combo_at) {
					auto& entry {subs.cell_tags[combo_walker.combo_at(static_cast<o2x_t>(naked_subset_size-1-combo_at))]};
					std::swap(subs.cell_tags[static_cast<o2x_t>(sub_z-1)], entry);
					--sub_z;
				}
				subs.is_begin.set(static_cast<o2x_t>(sub_z));
			}
			for (auto i {sub_a}; i < sub_z; ++i) {
				const auto check {engine.do_elim_remove_syms_(subs.cell_tags[i].rmi, combo_syms)};
				if (check.did_unwind()) [[unlikely]] { return check; }
			}
			if (naked_or_hidden != NakedOrHidden::naked) {
				sub_a = sub_z;
			}
			engine.get_found_queues_().push_back(found::Subset<O>{}); // TODO currently pushing a dummy desc just to get proper finder looping in FastSolver
		}
		return UnwindInfo::make_no_unwind();
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo find_subsets_and_check_needs_unwind(
		EngineImpl<O>& engine,
		const int_ts::o2x_t<O> max_subset_size
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		OKIIDOKU_CONTRACT_USE(max_subset_size <= static_cast<o2x_t>((T::O2+1)/2));
		if (max_subset_size < 2) [[unlikely]] {
			return UnwindInfo::make_no_unwind();
		}
		for (const auto house_type : house_types) {
			auto& houses_subsets {engine.houses_subsets().at(house_type)};
			// std::array<o2i_t, T::O2> houses;
			// std::iota(houses.begin(), houses.end(), o2i_t{0});
			// std::sort(houses.begin(), houses.end(), [&](const auto& a, const auto& b){
			// 	return houses_subsets[a].is_begin.count() < houses_subsets[b].is_begin.count();
			// });
			// for (const auto house : houses) {
			for (o2i_t house {0}; house < T::O2; ++house) {
				const auto check {find_subsets_for_house_and_check_needs_unwind<O>(
					engine, houses_subsets[house], max_subset_size
				)};
				if (check.did_unwind()) [[unlikely]] { return check; }
			}
		}
		return UnwindInfo::make_no_unwind();
	}
}}
namespace okiidoku::mono::detail::solver {

	OKIIDOKU_CAND_ELIM_FINDER_DEF(sym_claim_cell)
	#undef OKIIDOKU_CAND_ELIM_FINDER

	template<Order O> requires(is_order_compiled(O)) \
	UnwindInfo CandElimFind<O>::subsets(Engine<O>& engine, const int_ts::o2x_t<O> max_subset_size) noexcept {
		OKIIDOKU_CONTRACT_ASSERT(!engine.no_more_solns());
		if (engine.get_num_unsolved() == 0) [[unlikely]] { return UnwindInfo::make_no_unwind(); }
		return find_subsets_and_check_needs_unwind(engine, max_subset_size);
	}

	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template UnwindInfo CandElimFind<O_>::sym_claim_cell(Engine<O_>&) noexcept; \
		template UnwindInfo CandElimFind<O_>::subsets(Engine<O_>&, int_ts::o2x_t<O_> max_subset_size) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}