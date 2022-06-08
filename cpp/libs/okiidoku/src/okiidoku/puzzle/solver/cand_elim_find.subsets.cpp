#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

#include <okiidoku/puzzle/solver/subset_combo_walker.hpp>
#include <okiidoku/puzzle/solver/found.hpp>
#include <okiidoku/o2_bit_arr.hpp>

#include <functional> // function, bit_or
#include <algorithm> // find_if, transform_reduce
#include <execution>
#include <array>
#include <compare>

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
	[[nodiscard]] bool helper_find_and_check_needs_unwind(
		typename EngineImpl<O>::CandsGrid& cells_cands,
		typename EngineImpl<O>::house_subset_clusters_t& clusters
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		auto cluster_begin {clusters.begin()};
		auto cluster_end {cluster_begin};
		const auto update_cluster_end {[&]{
			cluster_end = std::find_if(std::next(cluster_begin), clusters.end(), [&](const auto& e){
				return e.is_cluster_begin;
			});
		}};
		update_cluster_end();
		auto subset_size {static_cast<o2x_t>(2)};

		while (cluster_begin != clusters.end()) {
			assert(std::distance(cluster_begin, cluster_end) > 0);
			assert(cluster_begin->is_cluster_begin);
			assert(std::none_of(std::next(cluster_begin), cluster_end, [](const auto& e){ return e.is_cluster_begin; }));
			{
				// sort to enable the `cluster_sub_end` and size-one-cluster-skip optimizations.
				cluster_begin->is_cluster_begin = false;
				std::sort(cluster_begin, cluster_end, [&](const auto& a, const auto& b){
					return cells_cands.at_rmi(a.rmi).count() < cells_cands.at_rmi(b.rmi).count();
				});
				cluster_begin->is_cluster_begin = true;
			}
			// size-one-cluster-skip optimization:
			if (cells_cands.at_rmi(cluster_begin->rmi).count() == 1) [[unlikely]] {
				do {
					++cluster_begin;
					cluster_begin->is_cluster_begin = true;
				} while (cells_cands.at_rmi(cluster_begin->rmi).count() == 1);
				update_cluster_end();
				continue;
			}
			// check if no more subset sizes to search:
			if (subset_size+1 >= std::distance(cluster_begin, cluster_end)) {
				// ^plus one to skip finding hidden singles. // TODO or also try to find them?
				subset_size = 2;
				cluster_begin = cluster_end;
				update_cluster_end();
				continue;
			}
			const auto cluster_sub_end {std::find_if(cluster_begin, cluster_end, [&](const auto& e){
				return cells_cands.at_rmi(e.rmi).count() > subset_size;
			})};
			SubsetComboWalker<O> combo_walker {
				static_cast<o2x_t>(std::distance(clusters.begin(), cluster_begin)),
				static_cast<o2i_t>(std::distance(clusters.begin(), cluster_sub_end)),
				subset_size,
			};
			O2BitArr<O> combo_syms {};
			for (; combo_walker.has_more(); combo_walker.advance()) {
				combo_syms = std::transform_reduce(
					std::execution::unseq,
					combo_walker.at_it(), std::next(combo_walker.at_it(), subset_size),
					O2BitArr<O>{}, std::bit_or{}, [&](const auto i) -> auto& {
						return cells_cands.at_rmi(clusters[i].rmi);
					}
				);
				if (combo_syms.count() <  subset_size) [[unlikely]] { return true; }
				if (combo_syms.count() == subset_size) [[unlikely]] { break; }
			}
			if (combo_syms.count() == subset_size) [[unlikely]] {
				const auto old_cluster_begin {cluster_begin};
				old_cluster_begin->is_cluster_begin = false;
				for (o2x_t i {0}; i < subset_size; ++i) {
					auto& entry {clusters[combo_walker.combo_at(i)]};
					std::swap(*cluster_begin, entry);
					++cluster_begin;
				}
				old_cluster_begin->is_cluster_begin = true;
				cluster_begin->is_cluster_begin = true;
				for (auto it {cluster_begin}; it != cluster_end; ++it) {
					auto& other {cells_cands.at_rmi(it->rmi)};
					const auto old_count {other.count()};
					other.remove(combo_syms);
					if (other.count() == 0) [[unlikely]] { return true; }
					if (other.count() == 1 && other.count() < old_count) [[unlikely]] {
						engine.enqueue_cand_elims_for_new_cell_claim_sym_(it->rmi);
					}
				}
				subset_size = 2;
			} else {
				++subset_size;
			}
		}
		return false; // no unwind needed.
	}


	template<Order O> requires(is_order_compiled(O))
	[[nodiscard]] bool find_cells_claim_syms_and_check_needs_unwind(
		const CandsGrid<O>& cells_cands,
		FoundQueues<O>& found_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		for (const auto house_type : house_types) {
		for (o2i_t house {0}; house < T::O2; ++house) {
			if (helper_find_and_check_needs_unwind<O>(
				engine.cells_cands, engine.houses_subset_clusters
			)) [[unlikely]] { return true; }
		}}
		return false;
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
	OKIIDOKU_CAND_ELIM_FINDER_DEF(cells_claim_syms)
	// OKIIDOKU_CAND_ELIM_FINDER_DEF(syms_claim_cells)
	#undef OKIIDOKU_CAND_ELIM_FINDER

	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template UnwindInfo CandElimFind<O_>::sym_claim_cell(Engine<O_>&) noexcept; \
		template UnwindInfo CandElimFind<O_>::cells_claim_syms(Engine<O_>&) noexcept; \
		// template UnwindInfo CandElimFind<O_>::syms_claim_cells(Engine<O_>&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}