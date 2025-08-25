// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/morph/rel_info.hpp>

#include <okiidoku/o2_bit_arr.hpp>

#include <algorithm>  // swap, sort, ranges::next_permutation
#include <functional> // greater
#include <cstdlib>    // abs

namespace okiidoku::mono { namespace {

	/** describes, for a pair of symbols occurring in a grid, boxes where the pair
	occurs in an atom. */
	template<Order O> requires(is_order_compiled(O))
	struct [[gnu::designated_init]] RelMasks final {
		O2BitArr<O> boxes_h; ///< bit `i` is set if in box `i`, the symbol pair cohabits a horizontal atom.
		O2BitArr<O> boxes_v; ///< bit `i` is set if in box `i`, the symbol pair cohabits a vertical atom.
	};
	// O^4 * O^2 * 2 bits
	// O^2 * O^2 * [2*log2(O)] foreach sym, foreach box, which boxatoms is it in?
	// O^2 * 2*O * O^2 - foreach sym, for each boxatom, in which boxes is it in that boxatom?

	// Returns a Gridlike where rows and columns index symbol pairs, and values
	// are masks indicating blocks where the pair occurs in an atom.
	// Does not include self-to-self relationship bit for main diagonal entries.
	template<Order O> requires(is_order_compiled(O))
	detail::Gridlike<O, RelMasks<O>> make_rel_masks_(const Grid<O>& grid) noexcept {
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		using sym_t = int_ts::o2is_t<O>;

		/**
		TODO.high for large grid sizes, this uses hundreds of KB of stack.
		This is not good, since stacks have limits defaulted by the OS. Find a
		way to reduce memory usage, or allocate on heap... It's possible
		to halve the current usage by abstracting over the property of
		diagonal symmetry, but unfortunately, other than that, the
		current representation is optimizing for speed in a way that is
		not _needlessly_ wasteful of memory. Well, actually, the RelMasks
		is 25% waste: between boxes_h and boxes_v, if one is set at bit
		index i, the other will always be unset. So technically the number
		of bits there can be reduced to a factor of log(3)/log(2)/2, (0.7925)
		but that comes at the cost of having to reconstruct the two masks
		when they are later used. That direction doesn't sound appealing. */

		detail::Gridlike<O, RelMasks<O>> masks {};
		// iterate over atoms in lines:
		for (o2i_t line {0}; line < T::O2; ++line) {
		for (o2i_t atom {0}; atom < T::O2; atom += T::O1) {
			// Go through all unique pairs in the atom:
			for (o1i_t i {0}; i < T::O1 - 1; ++i) {
			for (o1i_t j {static_cast<o1i_t>(i + 1)}; j < T::O1; ++j) {
				{ // boxrow
					const sym_t i_sym {grid.at(line, static_cast<o2i_t>(atom+i))};
					const sym_t j_sym {grid.at(line, static_cast<o2i_t>(atom+j))};
					const auto box {static_cast<o2x_t>(rmi_to_box<O>(line, atom))};
					masks.at(i_sym,j_sym).boxes_h.set(box);
					masks.at(j_sym,i_sym).boxes_h.set(box);
				}
				{ // boxcol
					const sym_t i_sym {grid.at(static_cast<o2i_t>(atom+i), line)};
					const sym_t j_sym {grid.at(static_cast<o2i_t>(atom+j), line)};
					const auto box {static_cast<o2x_t>(rmi_to_box<O>(atom, line))};
					masks.at(i_sym,j_sym).boxes_v.set(box);
					masks.at(j_sym,i_sym).boxes_v.set(box);
				}
		}}	}}
		return masks;
	}
}}
namespace okiidoku::mono::detail {

	template<Order O> requires(is_order_compiled(O))
	detail::Gridlike<O, Rel<O>> make_rel_table(const Grid<O>& grid_in) noexcept {
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		using rel_at_mask_t = O2BitArr<O>;
		using chute_imbalance_t = typename Rel<O>::chute_imbalance_t;

		const detail::Gridlike<O, RelMasks<O>> masks {make_rel_masks_<O>(grid_in)};
		OKIIDOKU_NO_PRE_INIT_AUTOVAR detail::Gridlike<O, Rel<O>> table; // uninitialized!
		for (o2i_t sym_a {0}; sym_a < T::O2; ++sym_a) {
		for (o2i_t sym_b {0}; sym_b < T::O2; ++sym_b) {
			const auto& mask {masks.at(sym_a,sym_b)};
			auto& rel {table.at(sym_a,sym_b)};
			if (sym_a == sym_b) {
				rel = {0,(T::O2/2),0,0};
				continue;
			}
			const rel_at_mask_t rel_at_mask {mask.boxes_h | mask.boxes_v};
			const auto count {rel_at_mask.count()}; // how many boxes these two symbols cohabit an atom in
			rel.count = static_cast<int_ts::o2is_t<O>>(count);
			rel.polar_count_lesser = static_cast<typename Rel<O>::polar_count_lesser_t>(std::min(mask.boxes_h.count(), mask.boxes_v.count()));

			OKIIDOKU_NO_PRE_INIT_AUTOVAR std::array<o1x_t, T::O1> h_chute_imbalance; // NOLINT(cppcoreguidelines-pro-type-member-init) initialized in following loop
			OKIIDOKU_NO_PRE_INIT_AUTOVAR std::array<o1x_t, T::O1> v_chute_imbalance; // NOLINT(cppcoreguidelines-pro-type-member-init) initialized in following loop
			for (o1i_t chute {0}; chute < T::O1; ++chute) {
				h_chute_imbalance[chute] = static_cast<o1x_t>((ChuteBoxMasks<O>::row[chute] & rel_at_mask).count());
				v_chute_imbalance[chute] = static_cast<o1x_t>((ChuteBoxMasks<O>::col[chute] & rel_at_mask).count());
			}
			std::sort(h_chute_imbalance.begin(), h_chute_imbalance.end(), std::greater{});
			std::sort(v_chute_imbalance.begin(), v_chute_imbalance.end(), std::greater{});
			rel.chute_imbalance_a = 0;
			rel.chute_imbalance_b = 0;
			for (o1i_t i {0}; i < T::O1; ++i) {
				const auto expected_count = static_cast<chute_imbalance_t>((count / T::O1) + ((i < count % T::O1) ? 1U : 0U));
				rel.chute_imbalance_a += static_cast<chute_imbalance_t>(std::abs(h_chute_imbalance[i] - expected_count));
				rel.chute_imbalance_b += static_cast<chute_imbalance_t>(std::abs(v_chute_imbalance[i] - expected_count));
			}
			if (rel.chute_imbalance_b > rel.chute_imbalance_a) {
				std::swap(rel.chute_imbalance_a, rel.chute_imbalance_b);
			}
			// Note: if fast lexicographical compare is needed later, shrink h_chute_imbalance
			// by looping and doing `digits == h_chute_imbalance[i++]; digits *= T::O1;`
		}}
		return table;
	}


	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template struct Rel<O_>; \
		template detail::Gridlike<O_, Rel<(O_)>> make_rel_table<O_>(const Grid<O_>&) noexcept ;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}
