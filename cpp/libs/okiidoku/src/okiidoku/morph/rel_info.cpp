// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/detail/pgo_use_check_needs_rebuild.hpp>
#include <okiidoku/morph/rel_info.hpp>

#include <okiidoku/o2_bit_arr.hpp>

#include <algorithm> // swap, sort, ranges::next_permutation
#include <numeric>   // abs

namespace okiidoku::mono { namespace {

	template<Order O> requires(is_order_compiled(O))
	struct RelMasks final {
		O2BitArr<O> boxes_h;
		O2BitArr<O> boxes_v;
	};

	// A structure to avoid duplicating information from the diagonal symmetry
	// of a symbol-pair-relation table with large entries.
	// make the upper right triangle of the bottom left quarter mirror along
	// the main diagonal and map to the bottom left triangle of the upper left quarter.
	// struct FoldedRelMasksTable
	// Rel& at(sym_a, sym_b):
	// first, if sym_a > sym_b, swap them.
	// then, if sym_a > ((O2+1)//2), do the mirror and hug:
	//  swap sym_a and sym_b and subtract (O2//2) from both.
	// as a bonus optimization, the width of the table can be decremented by decrementing sym_b
	//  in an else block after the `if sym_a > ((O2+1)//2)`. This is based on the assumption that
	//  the input sym_a must not be equal to sym_b.
	// can consider having template specializations for smaller orders to not use this space optimization.
	// if so, to hide the detail of setting a bit for both halves, make the at getter const, and
	// have a separate setter method.

	template<Order O> requires(is_order_compiled(O))
	detail::Gridlike<O, RelMasks<O>> make_rel_masks_(const Grid<O>& grid) noexcept {
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		using val_t = int_ts::o2is_t<O>;

		// TODO.high for large grid sizes, this uses hundreds of KB of stack.
		// stacks have limits defaulted by the OS. This is not good. Find a
		// way to reduce memory usage, or allocate on heap... It's possible
		// to halve the current usage by abstracting over the property of
		// diagonal symmetry, but unfortunately, other than that, the
		// current representation is optimizing for speed in a way that is
		// not _needlessly_ wasteful of memory. Well, actually, the RelMasks
		// is 25% waste: between boxes_h and boxes_v, if one is set at bit
		// index i, the other will always be unset. So technically the number
		// of bits there can be reduced to a factor of log(3)/log(2)/2, (0.7925)
		// but that comes at the cost of having to reconstruct the two masks
		// when they are later used. That direction doesn't sound appealing.

		// rows and columns are symbols. cells are masks indicating blocks
		// where the two symbols indicated by the row and col are in a same line.
		// Does not include self-to-self relationship bit for main diagonal entries.
		detail::Gridlike<O, RelMasks<O>> masks {};
		for (o2i_t line {0}; line < T::O2; ++line) {
		for (o2i_t atom {0}; atom < T::O2; atom += T::O1) {
			// Go through all unique pairs in the atom:
			for (o1i_t i {0}; i < T::O1 - 1; ++i) {
			for (o1i_t j {static_cast<o1i_t>(i + 1)}; j < T::O1; ++j) {
				{ // boxrow
					const val_t i_val {grid.at(line, static_cast<o2i_t>(atom+i))};
					const val_t j_val {grid.at(line, static_cast<o2i_t>(atom+j))};
					const auto box {static_cast<o2x_t>(rmi_to_box<O>(line, atom))};
					masks.at(i_val,j_val).boxes_h.set(box);
					masks.at(j_val,i_val).boxes_h.set(box);
				}
				{ // boxcol
					const val_t i_val {grid.at(static_cast<o2i_t>(atom+i), line)};
					const val_t j_val {grid.at(static_cast<o2i_t>(atom+j), line)};
					const auto box {static_cast<o2x_t>(rmi_to_box<O>(atom, line))};
					masks.at(i_val,j_val).boxes_v.set(box);
					masks.at(j_val,i_val).boxes_v.set(box);
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
		detail::Gridlike<O, Rel<O>> table; // uninitialized!
		for (o2i_t sym_a {0}; sym_a < T::O2; ++sym_a) {
		for (o2i_t sym_b {0}; sym_b < T::O2; ++sym_b) {
			const auto& mask {masks.at(sym_a,sym_b)};
			auto& rel {table.at(sym_a,sym_b)};
			if (sym_a == sym_b) {
				rel = {0,(T::O2/2),0,0};
				continue;
			}
			const rel_at_mask_t rel_at_mask {mask.boxes_h | mask.boxes_v};
			const auto count {rel_at_mask.count()};
			rel.count = static_cast<int_ts::o2is_t<O>>(count);
			rel.polar_count_lesser = static_cast<typename Rel<O>::polar_count_lesser_t>(std::min(mask.boxes_h.count(), mask.boxes_v.count()));

			std::array<chute_imbalance_t, T::O1> h_chute_imbalance; // NOLINT(cppcoreguidelines-pro-type-member-init) initialized in following loop
			std::array<chute_imbalance_t, T::O1> v_chute_imbalance; // NOLINT(cppcoreguidelines-pro-type-member-init) initialized in following loop
			for (o1i_t chute {0}; chute < T::O1; ++chute) {
				h_chute_imbalance[chute] = static_cast<chute_imbalance_t>((chute_box_masks<O>::row[chute] & rel_at_mask).count());
				v_chute_imbalance[chute] = static_cast<chute_imbalance_t>((chute_box_masks<O>::col[chute] & rel_at_mask).count());
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