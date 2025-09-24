// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/puzzle/solver/cand_elim_find.hpp>

#include <okiidoku/puzzle/solver/engine.hpp>
#include <okiidoku/puzzle/solver/found.hpp>
#include <okiidoku/o2_bit_arr.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

// #include <algorithm>
#include <array>
#include <utility> // forward

#include <okiidoku/puzzle/solver/cand_elim_find.macros.hpp>

namespace okiidoku::mono::detail::solver { namespace {

	template<Order O> requires(is_order_compiled(O))
	struct ChuteIsecsSyms {
	private:
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		std::array<O2BitArr<O>, T::O2> arr_ {};
	public:
		// inner dimension is for intersections in a line, outer dimension for intersections in a box.
		template<class Self> [[nodiscard, gnu::pure]]
		decltype(auto) at_isec(this Self&& self, const o2i_t isec_i) noexcept { return std::forward<Self>(self).arr_[isec_i]; }

		template<class Self> [[nodiscard, gnu::pure]]
		decltype(auto) at_isec(this Self&& self, const o1x_t box_isec_i, const o1x_t line_isec_i) noexcept {
			return std::forward<Self>(self).arr_[((T::O1*box_isec_i)+line_isec_i)];
		}
	};

	template<Order O> requires(is_order_compiled(O))
	using chute_house_syms_t = std::array<O2BitArr<O>, Ints<O>::O1>;

	// TODO.low consider using this to clean up the variable scoping in the finder.
	//  ex. lines_syms and boxes_syms are only used intermediately to create lines_syms_claiming_an_isec and boxes_syms_claiming_an_isec.
	// template<Order O> requires(is_order_compiled(O))
	// struct HouseIsecClaims {
	// 	chute_house_syms_t<O> in_line;
	// 	chute_house_syms_t<O> in_box;
	// };

	// TODO should be able to implement generalizations as fish except for intersections
	template<Order O> requires(is_order_compiled(O))
	void find_locked_cands_in_chute(
		const LineType line_type,
		const typename Ints<O>::o1x_t chute,
		const CandsGrid<O>& cells_cands,
		FoundQueues<O>& found_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		// TODO optimize by interleaving entries of syms_non_single and syms?
		ChuteIsecsSyms<O> chute_isecs_syms_non_single {}; // syms with multiple cand cells in each chute isec
		ChuteIsecsSyms<O> chute_isecs_syms {}; // all cand syms in each chute isec
		for (const auto chute_isec : T::O2) {
		for (const auto isec_cell : T::O1) {
			const auto chute_cell {(T::O1*chute_isec)+isec_cell};
			const auto rmi {chute_cell_to_rmi<O>(line_type, chute, chute_cell)};
			const auto& cell_cands {cells_cands[rmi]};
			auto& seen_once {chute_isecs_syms.at_isec(chute_isec)};
			auto& seen_twice {chute_isecs_syms_non_single.at_isec(chute_isec)};
			seen_twice |= (seen_once & cell_cands);
			seen_once |= cell_cands;
		}}
		chute_house_syms_t<O> lines_syms_claiming_an_isec; lines_syms_claiming_an_isec.fill(O2BitArr_ones<O>); // an entry for each line indicating which syms only occur in one isec in the line.
		chute_house_syms_t<O> boxes_syms_claiming_an_isec; boxes_syms_claiming_an_isec.fill(O2BitArr_ones<O>); // an entry for each box indicating which syms only occur in one isec in the box.
		{
			chute_house_syms_t<O> lines_syms;
			chute_house_syms_t<O> boxes_syms;
			// TODO.low compare speed and lib-size if the initialization is not interleaved.
			for (const auto isec_i : T::O1) {
			for (const auto isec_j : T::O1) {{
				const auto& line_isec_syms {chute_isecs_syms.at_isec(isec_i, isec_j)};
				lines_syms_claiming_an_isec[isec_i].remove(lines_syms[isec_i] & line_isec_syms);
				lines_syms[isec_i] |= line_isec_syms;
				}{
				const auto& box_isec_syms {chute_isecs_syms.at_isec(isec_j, isec_i)};
				boxes_syms_claiming_an_isec[isec_i].remove(boxes_syms[isec_i] & box_isec_syms);
				boxes_syms[isec_i] |= box_isec_syms;
			}}}
		}
		for (const auto box_isec : T::O1) {
		for (const auto line_isec : T::O1) {
			const auto& isec_syms_non_single {chute_isecs_syms_non_single.at_isec(box_isec, line_isec)};
			auto line_match {lines_syms_claiming_an_isec[box_isec] & isec_syms_non_single}; line_match.remove(boxes_syms_claiming_an_isec[line_isec]);
			auto box_match {boxes_syms_claiming_an_isec[line_isec] & isec_syms_non_single};  box_match.remove(lines_syms_claiming_an_isec[box_isec]);
			const o3xs_t isec {
				(T::O2*chute)
				+ (T::O1*box_isec)
				+ line_isec
			};
			if (line_match.count() > 0) [[unlikely]] {
				found_queues.push_back(found::LockedCands<O>{
					.syms {line_match},
					.isec {isec},
					.line_type {line_type},
					.remove_from_rest_of {BoxOrLine::box},
				});
			}
			// Note: not an else-if: may be for different syms.
			if (box_match.count() > 0) [[unlikely]] {
				found_queues.push_back(found::LockedCands<O>{
					.syms {box_match},
					.isec {isec},
					.line_type {line_type},
					.remove_from_rest_of {BoxOrLine::line},
				});
			}
		}}
	}

	template<Order O> requires(is_order_compiled(O))
	[[nodiscard]] bool find_locked_cands_and_check_needs_unwind(
		const CandsGrid<O>& cells_cands,
		FoundQueues<O>& found_queues
	) noexcept {
		OKIIDOKU_CAND_ELIM_FINDER_TYPEDEFS
		// for intersection I of block B and line L, and symbol S,
		// if S's only candidate cells in L are in I, the same must hold true for B
		// if S's only candidate cells in B are in I, the same must hold true for L
		for (const auto line_type : line_types) {
			for (const auto chute : T::O1) {
				find_locked_cands_in_chute(line_type, chute, cells_cands, found_queues);
			}
		}
		return false;
	}
}}
namespace okiidoku::mono::detail::solver {

	OKIIDOKU_CAND_ELIM_FINDER_DEF(locked_cands)
	#undef OKIIDOKU_CAND_ELIM_FINDER_DEF

	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template UnwindInfo CandElimFind<O_>::locked_cands(Engine<O_>&) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}