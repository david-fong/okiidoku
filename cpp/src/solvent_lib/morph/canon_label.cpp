#include <solvent_lib/print.hpp> // TODO remove after done implementing
#include <solvent_lib/morph/rel_prob.hpp>
#include <solvent_lib/grid.hpp>

#include <iostream>
#include <algorithm> // swap, sort
#include <numeric>   // iota
#include <compare>   // partial_ordering
#include <bit>       // popcount

namespace solvent::lib::morph {

	template<Order O>
	class CanonLabel final {
	 static_assert(O > 0 && O < MAX_REASONABLE_ORDER);
		using has_mask_t = typename size<O>::O2_mask_least_t;
		using ord1_t = typename size<O>::ord1_t;
		using ord2_t = typename size<O>::ord2_t;
		using ord4_t = typename size<O>::ord4_t;
	 public:
		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;


		// Info not placement-independent.
		// Does not include self-to-self relationship bit for main diagonal entries.
		struct RelMask final {
			has_mask_t blocks_h = 0;
			has_mask_t blocks_v = 0;
		};
		static grid_mtx_t<O, RelMask> get_rel_masks_(const grid_const_span_t<O> grid_span) noexcept {
			grid_mtx_wrapper_t<O, const ord2_t> grid(grid_span);
			grid_mtx_t<O, RelMask> masks;
			for (ord2_t line = 0; line < O2; line++) {
				for (ord2_t atom = 0; atom < O2; atom += O1) {
					// Go through all unique pairs in the atom:
					for (ord1_t i = 0; i < O1 - 1; i++) {
						for (ord1_t j = i + 1; j < O1; j++) {
							{ // boxrow
								const ord2_t i_val = grid.at(line, atom+i), j_val = grid.at(line, atom+j);
								const has_mask_t blk_mask_bit = 1 << rmi2blk<O>(line, atom);
								masks[i_val][j_val].blocks_h |= blk_mask_bit;
								masks[j_val][i_val].blocks_h |= blk_mask_bit;
							}
							{ // boxcol
								const ord2_t i_val = grid.at(atom+i, line), j_val = grid.at(atom+j, line);
								const has_mask_t blk_mask_bit = 1 << rmi2blk<O>(atom, line);
								masks[i_val][j_val].blocks_v |= blk_mask_bit;
								masks[j_val][i_val].blocks_v |= blk_mask_bit;
							}
			}	}	}	}
			return masks;
		}


		// Info is placement-independent.
		struct RelCount final {
			ord2_t all, polar_a, polar_b;
			std::array<ord1_t, O1> all_chute_a_occ, all_chute_b_occ;
			[[gnu::pure]] double     all_prob() const noexcept { return RelCountProb<O>::ALL[all]; }
			[[gnu::pure]] double polar_a_prob() const noexcept { return RelCountProb<O>::POLAR[polar_a]; }
			[[gnu::pure]] double polar_b_prob() const noexcept { return RelCountProb<O>::POLAR[polar_b]; }
		};
		static grid_mtx_t<O, RelCount> get_rel_counts_(const grid_mtx_t<O, RelMask>& masks) noexcept {
			grid_mtx_t<O, RelCount> counts;
			for (ord2_t r = 0; r < O2; r++) { for (ord2_t c = 0; c < O2; c++) {
				const RelMask& mask = masks[r][c];
				RelCount& count = counts[r][c];
				const has_mask_t non_polar_mask = mask.blocks_h | mask.blocks_v;
				count.all     = std::popcount(non_polar_mask);
				count.polar_a = std::popcount(mask.blocks_h);
				count.polar_b = std::popcount(mask.blocks_v);

				for (ord1_t chute = 0; chute < O1; chute++) {
					count.all_chute_a_occ[chute] = std::popcount(chute_blk_masks<O>::row[chute] & non_polar_mask);
					count.all_chute_b_occ[chute] = std::popcount(chute_blk_masks<O>::col[chute] & non_polar_mask);
				}
				std::ranges::sort(count.all_chute_a_occ, std::greater{});
				std::ranges::sort(count.all_chute_b_occ, std::greater{});
			}}

			// normalize polar_a and polar_b:
			double polar_h_prob = 1.0, polar_v_prob = 1.0;
			for (const auto& row : counts) { for (const auto& count : row) {
				polar_h_prob *= count.polar_a_prob();
				polar_v_prob *= count.polar_b_prob();
			}}
			// TODO investigate whether ties happen below. If so, break by chute mask counts.
			if (polar_h_prob == polar_v_prob) {
				std::cout << "! tie between polar_h_prob and polar_v_prob." << std::endl;
			}
			if ((polar_h_prob > polar_v_prob) || false/* TODO */) {
				std::swap(polar_h_prob, polar_v_prob);
				for (auto& row : counts) { for (auto& count : row) {
					std::swap(count.polar_a, count.polar_b);
					std::swap(count.all_chute_a_occ, count.all_chute_b_occ);
				}}
			}
			return counts;
		}


		// For sorting label pair relations
		struct RelAccumProb final {
			// a is rarer on average compared to b (due to rel_count pass).
			double all, polar_a, polar_b;
			[[gnu::pure]] std::partial_ordering operator<=>(const RelAccumProb& that) const {
				std::partial_ordering cmp = all <=> that.all;
				if (cmp != std::partial_ordering::equivalent) [[likely]] { return cmp; }
				cmp = polar_a <=> that.polar_a;
				if (cmp != std::partial_ordering::equivalent) [[likely]] { return cmp; }
				return polar_b <=> that.polar_b;
			}
		};
		static grid_mtx_t<O, RelAccumProb> get_rel_cmp_layers_(const grid_mtx_t<O, RelCount>& counts) noexcept {
			// sort, accumulate/ramp, transpose (for caching optimization)
			grid_mtx_t<O, RelAccumProb> pre_transpose;
			for (ord2_t i = 0; i < O2; i++) { for (ord2_t j = 0; j < O2; j++) {
				const auto count = counts[i][j];
				pre_transpose[i][j] = RelAccumProb {
					.all = count.all_prob(), .polar_a = count.polar_a_prob(), .polar_b = count.polar_b_prob()
				};
			}}
			for (auto& row : pre_transpose) {
				// put rare things at the back:
				std::ranges::sort(row, std::greater{});
			}
			for (auto& row : pre_transpose) {
				for (ord2_t i = O2-1; i > 0; i--) {
					// Iterating forward progressively removes non-rare items:
					row[i-1].all *= row[i].all;
					row[i-1].polar_a *= row[i].polar_a;
					row[i-1].polar_b *= row[i].polar_b;
			}	}
			grid_mtx_t<O, RelAccumProb> cmp_layers;
			for (ord2_t i = 0; i < O2; i++) { for (ord2_t j = 0; j < O2; j++) {
				cmp_layers[i][j] = pre_transpose[j][i];
			}}
			return cmp_layers;
		}


		//
		static grid_vec_t<O> do_it(const grid_const_span_t<O> orig_grid) {
			const std::array<ord2_t, O2> label_orig2canon = [&orig_grid](){
				std::array<ord2_t, O2> label_canon2orig;
				std::iota(label_canon2orig.begin(), label_canon2orig.end(), 0);

				const auto rel_cmp_layers = get_rel_cmp_layers_(
					get_rel_counts_(
						get_rel_masks_(orig_grid)
					)
				);
				std::sort(label_canon2orig.begin(), label_canon2orig.end(), [&rel_cmp_layers](const ord2_t& a_orig_label, const ord2_t& b_orig_label){
					for (ord2_t layer = 0; layer < O2; layer++) {
						const auto& a = rel_cmp_layers[layer][a_orig_label];
						const auto& b = rel_cmp_layers[layer][b_orig_label];
						if (a.all != b.all) [[likely]] { return a.all < b.all; }
						if (a.polar_a != b.polar_a) [[likely]] { return a.polar_a < b.polar_a; }
						if (a.polar_b != b.polar_b) [[likely]] { return a.polar_b < b.polar_b; }
					}
					std::cout << "! equivalent rel encountered." << std::endl;
					return false;
				});
				std::array<ord2_t, O2> _;
				for (ord2_t i_canon = 0; i_canon < O2; i_canon++) {
					_[label_canon2orig[i_canon]] = i_canon;
				}
				return _;
			}();
			grid_vec_t<O> canon_grid_vec(O4);
			for (ord4_t i = 0; i < O4; i++) {
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