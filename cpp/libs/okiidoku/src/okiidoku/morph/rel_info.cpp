#include <okiidoku/morph/rel_info.hpp>

#include <algorithm> // swap, sort, ranges::next_permutation
#include <numeric>   // abs

namespace okiidoku::mono::morph {

	// Info is placement-independent.
	// Does not include self-to-self relationship bit for main diagonal entries.
	template<Order O>
	struct RelMasks final {
		typename traits<O>::o2_bits_smol boxes_h;
		typename traits<O>::o2_bits_smol boxes_v;
	};
	template<Order O>
	GridArr<O, RelMasks<O>> make_rel_masks_(const GridConstSpan<O> grid) noexcept {
		using T = traits<O>;
		using has_mask_t = T::o2_bits_smol;
		using val_t = T::o2i_smol_t;
		using o1i_t = T::o1i_t;
		using o2i_t = T::o2i_t;

		GridArr<O, RelMasks<O>> masks {};
		for (o2i_t line {0}; line < T::O2; ++line) {
		for (o2i_t atom {0}; atom < T::O2; atom += T::O1) {
			// Go through all unique pairs in the atom:
			for (o1i_t i {0}; i < T::O1 - 1; ++i) {
			for (o1i_t j = i + 1; j < T::O1; ++j) {
				{ // boxrow
					const val_t i_val = grid.at(line, atom+i), j_val = grid.at(line, atom+j);
					const has_mask_t box_mask_bit = has_mask_t{1} << rmi_to_box<O>(line, atom);
					masks.at(i_val,j_val).boxes_h |= box_mask_bit;
					masks.at(j_val,i_val).boxes_h |= box_mask_bit;
				}
				{ // boxcol
					const val_t i_val = grid.at(atom+i, line), j_val = grid.at(atom+j, line);
					const has_mask_t box_mask_bit = has_mask_t{1} << rmi_to_box<O>(atom, line);
					masks.at(i_val,j_val).boxes_v |= box_mask_bit;
					masks.at(j_val,i_val).boxes_v |= box_mask_bit;
				}
		}}	}}
		return masks;
	}


	template<Order O>
	requires (is_order_compiled(O))
	GridArr<O, Rel<O>> make_rel_table(const GridConstSpan<O> grid_in) {
		using T = traits<O>;
		using has_mask_t = T::o2_bits_smol;
		using o1i_t = T::o1i_t;
		using o2i_t = T::o2i_t;
		using chute_imbalance_t = chute_imbalance_t<O>;

		const GridArr<O, RelMasks<O>> masks = make_rel_masks_<O>(grid_in);
		GridArr<O, Rel<O>> table; // uninitialized!
		for (o2i_t r {0}; r < T::O2; ++r) {
		for (o2i_t c {0}; c < T::O2; ++c) {
			const auto& mask = masks.at(r,c);
			auto& rel = table.at(r,c);
			if (r == c) {
				rel = {0,(T::O2/2),0,0};
				continue;
			}
			const has_mask_t non_polar_mask = mask.boxes_h | mask.boxes_v;
			const unsigned count = non_polar_mask.count();
			rel.count = static_cast<T::o2i_smol_t>(count);
			rel.polar_count_lesser = static_cast<Rel<O>::polar_count_lesser_t>(std::min(mask.boxes_h.count(), mask.boxes_v.count()));

			std::array<chute_imbalance_t, T::O1> h_chute_imbalance;
			std::array<chute_imbalance_t, T::O1> v_chute_imbalance;
			for (o1i_t chute {0}; chute < T::O1; ++chute) {
				h_chute_imbalance[chute] = static_cast<chute_imbalance_t>((chute_box_masks<O>::row[chute] & non_polar_mask).count());
				v_chute_imbalance[chute] = static_cast<chute_imbalance_t>((chute_box_masks<O>::col[chute] & non_polar_mask).count());
			}
			std::ranges::sort(h_chute_imbalance, std::greater{});
			std::ranges::sort(v_chute_imbalance, std::greater{});
			rel.chute_imbalance_a = 0;
			rel.chute_imbalance_b = 0;
			for (o1i_t i {0}; i < T::O1; ++i) {
				const chute_imbalance_t expected_count = static_cast<chute_imbalance_t>((count / T::O1) + ((i < count % T::O1) ? 1 : 0));
				rel.chute_imbalance_a += static_cast<chute_imbalance_t>(std::abs(h_chute_imbalance[i] - expected_count));
				rel.chute_imbalance_b += static_cast<chute_imbalance_t>(std::abs(v_chute_imbalance[i] - expected_count));
			}
			if (rel.chute_imbalance_b > rel.chute_imbalance_a) {
				std::swap(rel.chute_imbalance_a, rel.chute_imbalance_b);
			}
			// note: if fast lexicographical compare is needed later, shrink h_chute_imbalance
			// by looping and doing `digits == h_chute_imbalance[i++]; digits *= T::O1;`
		}}
		{
			// normalize polar fields
			// long double h_p = 1.0, v_p = 1.0;
			// o6i_t h_occ_dev {0}, v_occ_dev {0};
			// for (const auto& row : table) { for (const Rel& rel : row) {
			// 	h_p *= rel.polar_a_p;
			// 	v_p *= rel.polar_b_p;
			// 	h_occ_dev += rel.h_chute_imbalance;
			// 	v_occ_dev += rel.v_chute_imbalance;
			// }}
			// const auto cmp_p = h_p <=> v_p;
			// const auto cmp_occ_dev = h_occ_dev <=> v_occ_dev;
			// if (/* h is less rare */std::is_gt(cmp_p)
			// || (std::is_eq(cmp_p) && /* v deviates more */std::is_lt(cmp_occ_dev))
			// ) {
			// 	std::swap(h_p, v_p);
			// 	for (auto& row : table) { for (auto& count : row) {
			// 		std::swap(count.polar_a_p, count.polar_b_p);
			// 		std::swap(count.h_chute_imbalance, count.v_chute_imbalance);
			// 	}}
			// } else if (std::is_eq(cmp_p) && std::is_eq(cmp_occ_dev)) {
			// 	std::clog << "\n! tie between all rel polar stats.";
			// }
		}
		return table;
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template struct Rel<O_>; \
		template GridArr<O_, Rel<O_>> make_rel_table<O_>(GridConstSpan<O_>);
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}