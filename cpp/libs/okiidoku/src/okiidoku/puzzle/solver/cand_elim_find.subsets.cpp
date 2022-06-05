#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

#include <okiidoku/puzzle/solver/subset_combo_walker.hpp>
#include <okiidoku/puzzle/solver/found.hpp>
#include <okiidoku/o2_bit_arr.hpp>

#include <range/v3/view/take.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/algorithm/fold.hpp>

#include <functional> // function, bit_or
#include <algorithm>
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
					if (cell_cands.count() > 1) {
						found_queues.push_back(found::SymClaimCell<O>{
							.rmi{static_cast<rmi_t>(rmi)},
							.val{static_cast<o2xs_t>(sym)},
						});
				}	}
			}
		}}
		return false; // no unwind needed.
	}


	// Note: from sudopedia: the hidden/naked subsets of size greater than
	//  (O2+1)//2 have complements of the other kind of smaller subset size.
	//  I am not using this property for any optimization because my finder
	//  for subsets searches for all subset-sizes in a single pass.
	// template<Order O> requires(is_order_compiled(O))
	// static constexpr unsigned max_subset_size {(Ints<O>::O2 + 1U) / 2U};

	template<Order O> requires(is_order_compiled(O))
	struct GroupMe final {
		O2BitArr<O> cands;
		int_ts::o2is_t<O> cand_count;
		int_ts::o2xs_t<O> who;
		[[nodiscard, gnu::pure]] friend auto operator<=>(const GroupMe& a, const GroupMe& b) noexcept {
			return a.cand_count <=> b.cand_count;
		}
	};

	template<Order O> requires(is_order_compiled(O))
	using enqueue_found_fn_t = std::function<void(
		const O2BitArr<O>& what_required,
		const O2BitArr<O>& who_requires
	)>;

	template<Order O> requires(is_order_compiled(O))
	[[nodiscard]] bool helper_find_and_check_needs_unwind(
		std::array<GroupMe<O>, Ints<O>::O2>& set,
		enqueue_found_fn_t<O> enqueue_found_fn
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		std::sort(set.begin(), set.end());

		// find all combinations of N sets where their union is size N.
		// given that any one set can only be part of one such finding.
		auto search_begin {ranges::find_if(set, [](const auto& gm){ return gm.cand_count > 1; })};
		auto search_end {search_begin};
		for (o2x_t subset_size {2}; subset_size <= (std::distance(search_begin, set.end())/2U); ++subset_size) {
			search_end = ranges::find_if(search_begin, set.end(), [&](const auto& gm){
				return gm.cand_count > subset_size;
			});
			if (std::distance(search_begin, search_end) < subset_size) { continue; }
			SubsetComboWalker<O> combo_walker {
				static_cast<o2x_t>(std::distance(set.begin(), search_begin)),
				static_cast<o2i_t>(std::distance(set.begin(), search_end)),
				subset_size,
			};
			for (; combo_walker.has_more(); combo_walker.advance()) {
				const auto combo_union {*ranges::fold_left_first(
					ranges::views::take(combo_walker.get_combo_arr(), subset_size)
					| ranges::views::transform([&](const auto i){ return set[i].cands; })
				, std::bit_or{})};
				if (combo_union.count() < subset_size) [[unlikely]] {
					return true; // needs unwind.
				}
				if (combo_union.count() > subset_size) [[likely]] {
					continue;
				}
				O2BitArr<O> who_requires {};
				for (o2x_t i {0}; i < subset_size; ++i) {
					auto& member {set[combo_walker.combo_at(i)]};
					who_requires.set(member.who);
					assert(search_begin != set.end());
					std::swap(*search_begin, member);
					++search_begin;
				}
				for (auto it {search_begin}; it != set.end(); ++it) {
					it->cands.remove(combo_union);
					it->cand_count = static_cast<o2is_t>(it->cands.count());
					if (it->cand_count == 0) [[unlikely]] { return true; }
					// ^short-circuit not really necessary. will be detected later during
					//  apply. last I benchmarked on O=3, only had a 1% time improvement.
				}
				std::sort(search_begin, set.end());
				search_begin = ranges::find_if(search_begin, set.end(), [](const auto& gm){ return gm.cand_count > 1; });
				enqueue_found_fn(combo_union, who_requires);
				subset_size = 2;
				break; // exit combo_walker loop.
		}	}
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
			std::array<GroupMe<O>, T::O2> set;
			for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
				const auto& cell_cands {cells_cands.at_rmi(house_cell_to_rmi<O>(house_type, house, house_cell))};
				set[house_cell] = GroupMe<O>{
					.cands{cell_cands},
					.cand_count{static_cast<o2is_t>(cell_cands.count())},
					.who{static_cast<o2xs_t>(house_cell)},
				};
			}
			if (helper_find_and_check_needs_unwind<O>(set, [&](const auto& what, const auto& who){
				found_queues.push_back(found::CellsClaimSyms<O>{
					what, who, static_cast<o2xs_t>(house), house_type
				});
			})) { return true; }
		}}
		return false;
	}


	template<Order O> requires(is_order_compiled(O))
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
			if (helper_find_and_check_needs_unwind<O>(set, [&](const auto& what, const auto& who) noexcept {
				found_queues.push_back(found::SymsClaimCells<O>{
					what, who, static_cast<o2xs_t>(house), house_type
				});
			})) { return true; }
		}}
		return false;
	}
}}
namespace okiidoku::mono::detail::solver {

	OKIIDOKU_CAND_ELIM_FINDER_DEF(sym_claim_cell)
	OKIIDOKU_CAND_ELIM_FINDER_DEF(cells_claim_syms)
	OKIIDOKU_CAND_ELIM_FINDER_DEF(syms_claim_cells)
	#undef OKIIDOKU_CAND_ELIM_FINDER

	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template UnwindInfo CandElimFind<O_>::sym_claim_cell(Engine<O_>&) noexcept; \
		template UnwindInfo CandElimFind<O_>::cells_claim_syms(Engine<O_>&) noexcept; \
		template UnwindInfo CandElimFind<O_>::syms_claim_cells(Engine<O_>&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}