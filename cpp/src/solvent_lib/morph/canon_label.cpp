#include <solvent_lib/grid.hpp>
#include <solvent_lib/morph/rel_prob.hpp>

#include <iostream> // TODO remove
#include <algorithm> // swap, sort
#include <numeric>   // iota, abs
#include <compare>   // partial_ordering, is_eq, etc.
#include <bit>       // popcount
#include <cassert>

namespace solvent::lib::morph {

	template<Order O>
	class CanonLabel final {
	 static_assert((O > 0) && (O <= O_MAX));
		using has_mask_t = size<O>::O2_mask_least_t;
		using ord1i_t = size<O>::ord1i_t;
		using ord2i_t = size<O>::ord2i_t;
		using ord2x_t = size<O>::ord2x_t;
		using ord4i_t = size<O>::ord4i_t;
		using ord6i_t = size<O>::ord6i_t;
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
		static grid_arr_t<O, RelMask> make_rel_masks_(const grid_const_span_t<O> grid_span) noexcept {
			grid_span2d_t<O, const ord2i_t> grid(grid_span);
			grid_arr_t<O, RelMask> masks{}; // zero initialize
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
			ord2i_t all_chute_a_occ, all_chute_b_occ;

			std::partial_ordering operator<=>(const RelPlaceless& that) const {
				std::partial_ordering cmp = all_p <=> that.all_p;
				#define try_else(field) \
				if (cmp != std::partial_ordering::equivalent) [[likely]] { return cmp; }\
				cmp = field <=> that.field;
				// comments show percentage ties unbroken after try for O = 3.
				/* 50.0% */ try_else(polar_a_p)
				/* 5.00% */ try_else(polar_b_p)
				/* 1.80% */ try_else(all_chute_a_occ) // TODO this might be okay to take out for O>3? needs much more testing. in which case, use conditional_t for the struct fields
				/* 1.25% */ try_else(all_chute_b_occ)
				/* 0.95% */ return cmp;
			}
		};
		static grid_arr_t<O, RelPlaceless> make_rel_placeless_(const grid_arr_t<O, RelMask>& masks) noexcept {
			grid_arr_t<O, RelPlaceless> table;
			for (ord2i_t r = 0; r < O2; r++) { for (ord2i_t c = 0; c < O2; c++) {
				const RelMask& mask = masks[r][c];
				RelPlaceless& rel = table[r][c];
				const has_mask_t non_polar_mask = mask.blocks_h | mask.blocks_v;
				const int all_count = static_cast<int>(std::popcount(non_polar_mask));
				rel.all_p     = RelCountProb<O>::ALL[all_count];
				rel.polar_a_p = static_cast<float>(RelCountProb<O>::POLAR[std::popcount(mask.blocks_h)]);
				rel.polar_b_p = static_cast<float>(RelCountProb<O>::POLAR[std::popcount(mask.blocks_v)]);

				std::array<int8_t, O1> all_chute_a_occ, all_chute_b_occ;
				for (ord1i_t chute = 0; chute < O1; chute++) {
					all_chute_a_occ[chute] = static_cast<int8_t>(std::popcount(static_cast<has_mask_t>(chute_blk_masks<O>::row[chute] & non_polar_mask)));
					all_chute_b_occ[chute] = static_cast<int8_t>(std::popcount(static_cast<has_mask_t>(chute_blk_masks<O>::col[chute] & non_polar_mask)));
				}
				std::ranges::sort(all_chute_a_occ, std::less{});
				std::ranges::sort(all_chute_b_occ, std::less{});
				rel.all_chute_a_occ = 0;
				rel.all_chute_b_occ = 0;
				for (ord1i_t i = 0; i < O1; i++) {
					const int8_t expected_count = static_cast<int8_t>((all_count / O1) + ((i < all_count % O1) ? 1 : 0));
					rel.all_chute_a_occ += static_cast<ord2i_t>(std::abs(all_chute_a_occ[i] - expected_count));
					rel.all_chute_b_occ += static_cast<ord2i_t>(std::abs(all_chute_b_occ[i] - expected_count));
				}
				// note: if fast lexicographical compare is needed later, shrink all_chute_a_occ
				// by looping and doing `digits == all_chute_a_occ[i++]; digits *= O1;`
			}}
			{
				// normalize polar fields
				double h_p = 1.0, v_p = 1.0;
				ord6i_t h_occ_dev = 0, v_occ_dev = 0;
				for (const auto& row : table) { for (const RelPlaceless& rel : row) {
					h_p *= rel.polar_a_p;
					v_p *= rel.polar_b_p;
					h_occ_dev += rel.all_chute_a_occ;
					v_occ_dev += rel.all_chute_b_occ;
				}}
				const auto cmp_p = h_p <=> v_p;
				const auto cmp_occ_dev = h_occ_dev <=> v_occ_dev;
				if (/* h is less rare */std::is_gt(cmp_p)
				|| (std::is_eq(cmp_p) && /* v deviates more */std::is_lt(cmp_occ_dev))
				) {
					std::swap(h_p, v_p);
					for (auto& row : table) { for (auto& count : row) {
						std::swap(count.polar_a_p, count.polar_b_p);
						std::swap(count.all_chute_a_occ, count.all_chute_b_occ);
					}}
				} else if (std::is_eq(cmp_p) && std::is_eq(cmp_occ_dev)) {
					std::clog << "\n! tie between all rel polar stats.";
				}
			}
			return table;
		}


		static grid_arr_t<O, RelPlaceless> make_rel_cmp_layers_(const grid_arr_t<O, RelPlaceless>& table) noexcept {
			// sort, accumulate/ramp, transpose (for caching optimization)
			grid_arr_t<O, RelPlaceless> pre_transpose = table;
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
			// TODO do more benchmarking comparing perf with or without transpose.
			//   some rough benchmarking on O=4 seems to indicate some benefit.
			grid_arr_t<O, RelPlaceless> cmp_layers;
			for (ord2i_t i = 0; i < O2; i++) { for (ord2i_t j = 0; j < O2; j++) {
				cmp_layers[i][j] = pre_transpose[j][i];
			}}
			return cmp_layers;
		}


		//
		static void do_it(const grid_span_t<O> grid) {
			const std::array<ord2x_t, O2> label_orig2canon = [](const grid_arr_t<O, RelPlaceless> rel_cmp_layers){
				std::array<ord2x_t, O2> label_canon2orig;
				std::iota(label_canon2orig.begin(), label_canon2orig.end(), 0);

				std::sort(label_canon2orig.begin(), label_canon2orig.end(), [&rel_cmp_layers](const ord2x_t& a_orig_label, const ord2x_t& b_orig_label){
					for (ord2i_t layer = 0; layer < O2; layer++) {
						const RelPlaceless& a = rel_cmp_layers[layer][a_orig_label];
						const RelPlaceless& b = rel_cmp_layers[layer][b_orig_label];
						return (a <=> b) == std::partial_ordering::less;
					}
					std::clog << "\n! equivalent rel encountered.";
					return false;
				});
				std::array<ord2i_t, O2> _;
				for (ord2i_t i_canon = 0; i_canon < O2; i_canon++) {
					_[label_canon2orig[i_canon]] = i_canon;
				}
				return _;
			}(make_rel_cmp_layers_(make_rel_placeless_(make_rel_masks_(grid))));

			for (ord4i_t i = 0; i < O4; i++) {
				grid[i] = label_orig2canon[grid[i]];
			}
			assert(is_grid_valid<O>(grid));
		}
	};


	template<Order O>
	void canon_label(const grid_span_t<O> grid) {
		return CanonLabel<O>::do_it(grid);
	}


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template void canon_label<O_>(grid_span_t<O_>);
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}