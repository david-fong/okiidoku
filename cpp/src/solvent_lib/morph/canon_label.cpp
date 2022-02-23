#include <solvent_lib/grid.hpp>
#include <solvent_lib/morph/rel_prob.hpp>

#include <iostream>
#include <algorithm> // swap, sort
#include <numeric>   // iota
#include <compare>   // partial_ordering, is_eq, etc.
#include <bit>       // popcount

namespace solvent::lib::morph {

	template<Order O>
	class CanonLabel final {
	 static_assert(O > 0 && O < O_MAX);
		using has_mask_t = size<O>::O2_mask_least_t;
		using ord1i_t = size<O>::ord1i_t;
		using ord2i_t = size<O>::ord2i_t;
		using ord4i_t = size<O>::ord4i_t;
	 public:
		static constexpr ord1i_t O1 = O;
		static constexpr ord2i_t O2 = O*O;
		static constexpr ord4i_t O4 = O*O*O*O;


		// Info not placement-independent.
		// Does not include self-to-self relationship bit for main diagonal entries.
		struct RelMask final {
			has_mask_t blocks_h;
			has_mask_t blocks_v;
		};
		static grid_mtx_t<O, RelMask> make_rel_masks_(const grid_const_span_t<O> grid_span) noexcept {
			grid_mtx_wrapper_t<O, const ord2i_t> grid(grid_span);
			grid_mtx_t<O, RelMask> masks{}; // zero initialize
			for (ord2i_t line = 0; line < O2; line++) {
				for (ord2i_t atom = 0; atom < O2; atom += O1) {
					// Go through all unique pairs in the atom:
					for (ord1i_t i = 0; i < O1 - 1; i++) {
						for (ord1i_t j = i + 1; j < O1; j++) {
							{ // boxrow
								const ord2i_t i_val = grid.at(line, atom+i), j_val = grid.at(line, atom+j);
								const has_mask_t blk_mask_bit = 1 << rmi2blk<O>(line, atom);
								masks[i_val][j_val].blocks_h |= blk_mask_bit;
								masks[j_val][i_val].blocks_h |= blk_mask_bit;
							}
							{ // boxcol
								const ord2i_t i_val = grid.at(atom+i, line), j_val = grid.at(atom+j, line);
								const has_mask_t blk_mask_bit = 1 << rmi2blk<O>(atom, line);
								masks[i_val][j_val].blocks_v |= blk_mask_bit;
								masks[j_val][i_val].blocks_v |= blk_mask_bit;
							}
			}	}	}	}
			return masks;
		}


		// Info is placement-independent.
		struct RelPlaceless final {
			double all_p;
			float polar_a_p, polar_b_p;
			float all_chute_a_occ, all_chute_b_occ;

			std::partial_ordering operator<=>(const RelPlaceless& that) const {
				std::partial_ordering cmp = all_p <=> that.all_p;
				if (cmp != std::partial_ordering::equivalent) [[likely]] { return cmp; }
				cmp = polar_a_p <=> that.polar_a_p;
				if (cmp != std::partial_ordering::equivalent) [[likely]] { return cmp; }
				return polar_b_p <=> that.polar_b_p;
			}
		};
		static grid_mtx_t<O, RelPlaceless> make_rel_placeless_(const grid_mtx_t<O, RelMask>& masks) noexcept {
			grid_mtx_t<O, RelPlaceless> table;
			for (ord2i_t r = 0; r < O2; r++) { for (ord2i_t c = 0; c < O2; c++) {
				const RelMask& mask = masks[r][c];
				RelPlaceless& rel = table[r][c];
				const has_mask_t non_polar_mask = mask.blocks_h | mask.blocks_v;
				rel.all_p     = RelCountProb<O>::ALL[std::popcount(non_polar_mask)];
				rel.polar_a_p = static_cast<float>(RelCountProb<O>::POLAR[std::popcount(mask.blocks_h)]);
				rel.polar_b_p = static_cast<float>(RelCountProb<O>::POLAR[std::popcount(mask.blocks_v)]);

				std::array<ord1i_t, O1> all_chute_a_occ, all_chute_b_occ;
				for (ord1i_t chute = 0; chute < O1; chute++) {
					all_chute_a_occ[chute] = static_cast<ord1i_t>(std::popcount(chute_blk_masks<O>::row[chute] & non_polar_mask));
					all_chute_b_occ[chute] = static_cast<ord1i_t>(std::popcount(chute_blk_masks<O>::col[chute] & non_polar_mask));
				}
				std::ranges::sort(all_chute_a_occ, std::greater{});
				std::ranges::sort(all_chute_b_occ, std::greater{});
			}}
			{
				// normalize polar fields
				double h_p = 1.0, v_p = 1.0;
				// double h_occ_;
				for (const auto& row : table) { for (const auto& rel : row) {
					h_p *= rel.polar_a_p;
					v_p *= rel.polar_b_p;
				}}
				const auto p_cmp = h_p <=> v_p;
				if (std::is_gt(p_cmp) || (std::is_eq(p_cmp) && false) /* TODO */) {
					std::swap(h_p, v_p);
					for (auto& row : table) { for (auto& count : row) {
						std::swap(count.polar_a_p, count.polar_b_p);
						std::swap(count.all_chute_a_occ, count.all_chute_b_occ);
					}}
				} else {
					std::cout << "\n! tie between all rel polar stats.";
				}
			}
			return table;
		}


		static grid_mtx_t<O, RelPlaceless> make_rel_cmp_layers_(const grid_mtx_t<O, RelPlaceless>& table) noexcept {
			// sort, accumulate/ramp, transpose (for caching optimization)
			grid_mtx_t<O, RelPlaceless> pre_transpose = table;
			for (auto& row : pre_transpose) {
				// put rare things at the back:
				std::ranges::sort(row, std::greater{});
			}
			for (auto& row : pre_transpose) {
				for (ord2i_t i = O2-1; i > 0; i--) {
					// Iterating forward progressively removes non-rare items:
					row[i-1].all_p     *= row[i].all_p;
					row[i-1].polar_a_p *= row[i].polar_a_p;
					row[i-1].polar_b_p *= row[i].polar_b_p;
			}	}
			grid_mtx_t<O, RelPlaceless> cmp_layers;
			for (ord2i_t i = 0; i < O2; i++) { for (ord2i_t j = 0; j < O2; j++) {
				cmp_layers[i][j] = pre_transpose[j][i];
			}}
			return cmp_layers;
		}


		//
		static grid_vec_t<O> do_it(const grid_const_span_t<O> orig_grid) {
			const std::array<ord2i_t, O2> label_orig2canon = [](const grid_mtx_t<O, RelPlaceless> rel_cmp_layers){
				std::array<ord2i_t, O2> label_canon2orig;
				std::iota(label_canon2orig.begin(), label_canon2orig.end(), 0);

				std::sort(label_canon2orig.begin(), label_canon2orig.end(), [&rel_cmp_layers](const ord2i_t& a_orig_label, const ord2i_t& b_orig_label){
					for (ord2i_t layer = 0; layer < O2; layer++) {
						const RelPlaceless& a = rel_cmp_layers[layer][a_orig_label]; // TODO benchmark perf with vs without transpose. ie. does the transposition have more cost than its possible later benefit?
						const RelPlaceless& b = rel_cmp_layers[layer][b_orig_label];
						return (a <=> b) == std::partial_ordering::less;
					}
					std::cout << "\n! equivalent rel encountered.";
					return false;
				});
				std::array<ord2i_t, O2> _;
				for (ord2i_t i_canon = 0; i_canon < O2; i_canon++) {
					_[label_canon2orig[i_canon]] = i_canon;
				}
				return _;
			}(make_rel_cmp_layers_(make_rel_placeless_(make_rel_masks_(orig_grid))));

			grid_vec_t<O> canon_grid_vec(O4);
			for (ord4i_t i = 0; i < O4; i++) {
				canon_grid_vec[i] = label_orig2canon[orig_grid[i]];
			}
			return canon_grid_vec;
		}
	};


	template<Order O>
	grid_vec_t<O> canon_label(const grid_const_span_t<O> orig_grid) {
		return CanonLabel<O>::do_it(orig_grid);
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template grid_vec_t<O_> canon_label<O_>(grid_const_span_t<O_>);
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}