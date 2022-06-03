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
		for (auto house_type : house_types) {
		for (o2i_t house {0}; house < T::O2; ++house) {
			std::array<o2is_t, T::O2> syms_seen_only_once_at_house_cell;
			syms_seen_only_once_at_house_cell.fill(T::O2);
			O2BitArr<O> syms_seen {};
			O2BitArr<O> syms_seen_only_once {O2BitArr_ones<O>};
			for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
				const auto& cell_cands {cells_cands.at_rmi(house_cell_to_rmi<O>(house_type, house, house_cell))};
				syms_seen_only_once.remove(syms_seen & cell_cands);
				for (auto walker {(syms_seen_only_once & cell_cands).set_bits_walker()}; walker.has_more(); walker.advance()) {
					syms_seen_only_once_at_house_cell[walker.value()] = static_cast<o2is_t>(house_cell);
				}
				syms_seen |= cell_cands;
			}
			if (syms_seen.count() < T::O2) [[unlikely]] {
				return true; // needs unwind; some sym(s) have no cand cells.
			}
			for (auto walker {syms_seen_only_once.set_bits_walker()}; walker.has_more(); walker.advance()) {
				const auto rmi {house_cell_to_rmi<O>(house_type, house, syms_seen_only_once_at_house_cell[walker.value()])};
				if (cells_cands.at_rmi(rmi).count() > 1) {
					found_queues.push_back(found::SymClaimCell<O>{
						.rmi{static_cast<rmi_t>(rmi)},
						.val{static_cast<o2xs_t>(walker.value())},
					});
			}	}
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
	};

	template<Order O> requires(is_order_compiled(O))
	using enqueue_subset_desc_fn_t = std::function<void(
		const O2BitArr<O>& what_required,
		const O2BitArr<O>& who_requires
	)>;

	template<Order O> requires(is_order_compiled(O))
	[[nodiscard]] bool helper_find_and_check_needs_unwind(
		std::array<GroupMe<O>, Ints<O>::O2>& search_me,
		enqueue_subset_desc_fn_t<O> enqueue_subset_desc_fn
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		const auto group_me_cmp {[](const GroupMe<O>& a, const GroupMe<O>& b){
			if (const auto cmp {a.cand_count <=> b.cand_count}; std::is_neq(cmp)) [[likely]] { return cmp; }
			return O2BitArr<O>::cmp_differences(a.cands, b.cands); // TODO.try is this line beneficial? I actually don't even really know if it does what I intend, which is to put similar sets close together.
		}};
		std::sort(search_me.begin(), search_me.end(), [&](const auto& a, const auto& b){
			return std::is_lt(group_me_cmp(a,b));
		});
		// sort by count
		// for those with count 1, set count to 0 and get reduce union.
		// remove union from all other entries and update their count.
		// sort by count

		static constexpr o2x_t subset_size_begin {2U};
		for (o2x_t subset_size {2}; subset_size < T::O2; ++subset_size) {
			// TODO optimizations for this search
			const auto search_begin {ranges::find_if(search_me, [](const auto& gm){
				return gm.cand_count > 0;
			})};
			const auto search_end {ranges::find_if(search_begin, search_me.end(), [&](const auto& gm){
				return gm.cand_count > subset_size;
			})};
			if (std::distance(search_begin, search_end) < subset_size) { continue; }
			// find all combinations of N sets where their union is size N.
			// given that any one set can only be part of one such finding.
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
				, std::bit_or{})};
				if (combo_union.count() < subset_size) [[unlikely]] {
					return true; // needs unwind.
				}
				if (combo_union.count() == subset_size) [[unlikely]] {
					O2BitArr<O> who_requires {};
					for (o2x_t i {0}; i < subset_size; ++i) {
						who_requires.set(search_me[combo_walker.combo_at(i)].who);
					}
					enqueue_subset_desc_fn(combo_union, who_requires);
				}
			}
		}
		return false; // no unwind needed.
	}


	// TODO consider adding an argument about whether to do candidate elimination eagerly or queue them.
	//  the annoying thing is that then the return type for the queue case doesn't need to be UnwindInfo...
	//  in that case, we could make it a wrapper than just discards (void-casts) the result because it knows that it should never be UnwindInfo(false) if queueing. can also assert that that is so.
	//  the main question: why would that be beneficial? in the eager case, I think there's the benefit of operating on masks that
	//  are already in cache. but is that a significant enough benefit? probably needs benchmark to justify...
	//  The eager function wrapper takes an argument for find_one vs find_all. the queueing wrapper returns void.
	template<Order O> requires(is_order_compiled(O))
	[[nodiscard]] bool find_cells_claim_syms_and_check_needs_unwind(
		const CandsGrid<O>& cells_cands,
		FoundQueues<O>& found_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		for (auto house_type : house_types) {
		for (o2i_t house {0}; house < T::O2; ++house) {
			std::array<GroupMe<O>, T::O2> search_me;
			for (o2i_t house_cell {0}; house_cell < T::O2; ++house_cell) {
				const auto& cell_cands {cells_cands.at_rmi(house_cell_to_rmi<O>(house_type, house, house_cell))};
				search_me[house_cell] = GroupMe<O>{
					.cands{cell_cands},
					.cand_count{static_cast<o2is_t>(cell_cands.count())},
					.who{static_cast<o2xs_t>(house_cell)},
				};
			}
			if (helper_find_and_check_needs_unwind<O>(search_me, [&](const auto& what, const auto& who){
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
		for (auto house_type : house_types) {
		for (o2i_t house {0}; house < T::O2; ++house) {
			std::array<GroupMe<O>, T::O2> search_me {};
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
			if (helper_find_and_check_needs_unwind<O>(search_me, [&](const auto& what, const auto& who) noexcept {
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