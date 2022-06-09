#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

#include <okiidoku/puzzle/solver/subset_combo_walker.hpp>
#include <okiidoku/puzzle/solver/found.hpp>
#include <okiidoku/o2_bit_arr.hpp>

#include <functional> // bit_or
#include <algorithm> // find_if, transform_reduce
#include <execution>
#include <array>

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
					const auto sym {match_cands.count_lower_zeros_assuming_non_empty_mask()};
					if (cell_cands.count() > 1) /* [[?likely]] */ {
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
	UnwindInfo try_find_subset_for_size(
		EngineImpl<O>& engine,
		typename EngineImpl<O>::HouseSubsets& subs,
		int_ts::o2i_t<O>& sub_a,
		const int_ts::o2i_t<O> sub_z,
		int_ts::o2x_t<O>& subset_size
	) noexcept {
		assert(sub_a+subset_size+1 < sub_z);
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		auto& cells_cands {engine.cells_cands()};
		SubsetComboWalker<O> combo_walker {
			static_cast<o2x_t>(sub_a),
			[&]{
				for (auto i {sub_a}; i < sub_z; ++i) {
					if (cells_cands.at_rmi(subs.rmi[i]).count() > subset_size) { return i; }
				}
				return sub_z;
			}(),
			subset_size,
		};
		O2BitArr<O> combo_syms {};
		for (; combo_walker.has_more(); combo_walker.advance()) {
			combo_syms = std::transform_reduce(
				std::execution::unseq,
				combo_walker.at_it(),
				std::next(combo_walker.at_it(), subset_size),
				O2BitArr<O>{}, std::bit_or{}, [&](const auto i) -> const auto& {
					return cells_cands.at_rmi(subs.rmi[i]);
				}
			);
			if (combo_syms.count() <  subset_size) [[unlikely]] { return unwind_one_stack_frame_of_(engine); }
			if (combo_syms.count() == subset_size) [[unlikely]] { break; }
		}
		if (combo_syms.count() == subset_size) [[unlikely]] {
			for (o2x_t combo_at {0}; combo_at < subset_size; ++combo_at) {
				auto& entry {subs.rmi[combo_walker.combo_at(combo_at)]};
				std::swap(subs.rmi[sub_a], entry);
				++sub_a;
			}
			subs.is_begin.set(static_cast<o2x_t>(sub_a));
			for (auto i {sub_a}; i < sub_z; ++i) {
				const auto check {engine.do_elim_remove_syms_(subs.rmi[i], combo_syms)};
				if (check.did_unwind()) [[unlikely]] { return check; }
			}
			subset_size = 2;
			engine.get_found_queues_().push_back(found::CellsClaimSyms<O>{}); // TODO currently pushing a dummy desc just to get proper finder looping in FastSolver
		} else {
			++subset_size;
		}
		return UnwindInfo::make_no_unwind();
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo helper_find_and_check_needs_unwind(
		EngineImpl<O>& engine,
		typename EngineImpl<O>::HouseSubsets& subs
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		const auto& cells_cands {engine.cells_cands()};
		o2i_t sub_a {0};
		o2i_t sub_z {0};
		const auto update_sub_z {[&]{
			for (sub_z = sub_a+1; sub_z < T::O2 && !subs.is_begin.test(static_cast<o2x_t>(sub_z)); ++sub_z) {}
		}};
		o2x_t subset_size {2};

		while (sub_a < T::O2) {
			update_sub_z();
			assert(sub_z <= T::O2);
			assert(sub_a < sub_z);
			assert(subs.is_begin.test(static_cast<o2x_t>(sub_a)));
			#ifndef NDEBUG
			for (auto i {static_cast<o2i_t>(sub_a+1)}; i < sub_z; ++i) { assert(!subs.is_begin.test(static_cast<o2x_t>(i))); }
			#endif
			// sort to enable the `sub_sized_end` optimization and size-one-subset updates.
			std::sort(
				std::next(subs.rmi.begin(), static_cast<long>(sub_a)),
				std::next(subs.rmi.begin(), static_cast<long>(sub_z)),
				[&](const auto& rmi_a, const auto& rmi_b){
					return cells_cands.at_rmi(rmi_a).count() < cells_cands.at_rmi(rmi_b).count();
				}
			);
			// size-one-subset detect and update:
			if (cells_cands.at_rmi(subs.rmi[sub_a]).count() == 1) [[unlikely]] {
				do {
					++sub_a;
					if (sub_a == T::O2) [[unlikely]] { break; }
					subs.is_begin.set(static_cast<o2x_t>(sub_a));
				} while (sub_a < sub_z && cells_cands.at_rmi(subs.rmi[sub_a]).count() == 1);
				continue; // TODO optimize to skip sort when re-loop?
			}
			while (sub_a+subset_size+1 < sub_z) {
				// ^plus one to skip finding hidden singles. // TODO or also try to find them?
				const auto check {try_find_subset_for_size(engine, subs, sub_a, sub_z, subset_size)};
				if (check.did_unwind()) [[unlikely]] { return check; }
				if (subset_size == 2) [[unlikely]] {
					break;
				} // TODO currently ugly detection of successful subset find
			}
			if (!(sub_a+subset_size+1 < sub_z)) [[likely]] {
				subset_size = 2;
				sub_a = sub_z;
			}
		}
		return UnwindInfo::make_no_unwind();
	}


	template<Order O> requires(is_order_compiled(O))
	UnwindInfo find_cells_claim_syms_and_check_needs_unwind(
		EngineImpl<O>& engine
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		for (const auto house_type : house_types) {
		for (o2i_t house {0}; house < T::O2; ++house) {
			const auto check {helper_find_and_check_needs_unwind<O>(
				engine, engine.houses_subsets()[static_cast<unsigned char>(house_type)][house]
			)};
			if (check.did_unwind()) [[unlikely]] { return check; }
		}}
		return UnwindInfo::make_no_unwind();
	}


	/* template<Order O> requires(is_order_compiled(O))
	[[nodiscard]] bool find_syms_claim_cells_and_check_needs_unwind(
		const CandsGrid<O>& cells_cands,
		FoundQueues<O>& found_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		for (const auto house_type : house_types) {
		for (o2i_t house {0}; house < T::O2; ++house) {
			std::array<GroupMe<O>, T::O2> set {};
			// TODO.mid try applying loop-tiling to see if it improves cache-usage.
			for (o2i_t symbol {0}; symbol < T::O2; ++symbol) {
				auto& group_me {set[symbol]};
				group_me.who = static_cast<o2xs_t>(symbol);
				for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
					const auto rmi {house_cell_to_rmi<O>(house_type, house, house_cell)};
					if (cells_cands.at_rmi(rmi).test(static_cast<o2x_t>(symbol))) {
						group_me.cands.set(static_cast<o2x_t>(house_cell));
					}
				}
				group_me.cand_count = static_cast<o2is_t>(group_me.cands.count());
			}
			if (helper_find_and_check_needs_unwind<O>(set, [&](auto&& what, auto&& who) noexcept {
				found_queues.push_back(found::SymsClaimCells<O>{
					std::move(what), std::move(who), static_cast<o2xs_t>(house), house_type
				});
			})) { return true; }
		}}
		return false;
	} */
}}
namespace okiidoku::mono::detail::solver {

	OKIIDOKU_CAND_ELIM_FINDER_DEF(sym_claim_cell)
	OKIIDOKU_CAND_ELIM_FINDER_DEF_ALT(cells_claim_syms)
	// OKIIDOKU_CAND_ELIM_FINDER_DEF(syms_claim_cells)
	#undef OKIIDOKU_CAND_ELIM_FINDER

	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template UnwindInfo CandElimFind<O_>::sym_claim_cell(Engine<O_>&) noexcept; \
		template UnwindInfo CandElimFind<O_>::cells_claim_syms(Engine<O_>&) noexcept; \
		// template UnwindInfo CandElimFind<O_>::syms_claim_cells(Engine<O_>&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}