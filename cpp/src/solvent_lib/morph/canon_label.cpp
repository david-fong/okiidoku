#include <solvent_lib/print.hpp> // TODO remove after done implementing
#include <solvent_lib/morph/rel_prob.hpp>
#include <solvent_lib/grid.hpp>

#include <iostream>
#include <algorithm> // swap, sort
#include <numeric>   // transform_reduce
#include <execution> // execution::par_unseq
#include <cmath>     // pow
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
		struct RelMask {
			has_mask_t blocks_h;
			has_mask_t blocks_v;
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
		struct RelCount {
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
			if (polar_h_prob > polar_v_prob) {
				std::swap(polar_h_prob, polar_v_prob);
				for (auto& row : counts) { for (auto& count : row) {
					std::swap(count.polar_a, count.polar_b);
					std::swap(count.all_chute_a_occ, count.all_chute_b_occ);
				}}
			}
			return counts;
		}


		// For sorting label pair relations
		struct RelAccumProb {
			// a is rarer on average compared to b.
			double all, polar_a, polar_b;
			[[gnu::pure]] std::partial_ordering operator<=>(const RelAccumProb& that) const {
				std::strong_ordering cmp = all <=> that.all;
				if (cmp != std::partial_ordering::equivalent) [[likely]] { return cmp; }
				cmp = polar_a <=> that.polar_a;
				if (cmp != std::partial_ordering::equivalent) [[likely]] { return cmp; }
				return polar_b <=> that.polar_b;
			}
		};
		static grid_mtx_t<O, RelAccumProb> get_rel_accum_probs_(const grid_mtx_t<O, RelCount>& counts) noexcept {
			(void)counts; // TODO
			grid_mtx_t<O, RelAccumProb> accum_prob;
			return accum_prob;
		}


		grid_vec_t<O> operator()(const grid_const_span_t<O> orig_grid) {
			const auto rel_masks = get_rel_masks_(orig_grid);
			const auto rel_counts = get_rel_counts_(rel_masks);
			const auto rel_accum_probs = get_rel_accum_probs_(rel_counts);
			(void)rel_accum_probs; // TODO	
			grid_vec_t<O> grid(O4);
			return grid;
		}
	};


	template<Order O>
	grid_vec_t<O> canon_label(const grid_const_span_t<O> orig_grid) {
		return (CanonLabel<O>{})(orig_grid);
	}


	template<Order O>
	void canonicalize_labelling_(void) noexcept {
		// TODO
		/*
		struct SortMapEntry final {
			ord2_t orig; // The original label value
			double dist;
			double p_all;
			[[gnu::pure]] std::partial_ordering operator<=>(const SortMapEntry& that) const {
				// auto cmp = p_all <=> that.p_all;
				// if (cmp != std::partial_ordering::equivalent) [[likely]] { return cmp; }
				// return dist <=> that.dist;
				return p_all <=> that.p_all;
			}
		};
		// Make the lower-valued labels "play favourites":
		std::sort(canon2orig_label.begin(), canon2orig_label.end());
		// std::cout << "\n"; for (auto e : canon2orig_label) { std::cout << e.joint_prob << "  "; }
		{
			auto p_prev = canon2orig_label[0];
			for (ord2_t i = 1; i < O2; i++) {
				const auto p = canon2orig_label[i];
				// if (p.dist == p_prev.dist) [[unlikely]] {
				// 	rel_count_tie_mask_[i-1] = true; rel_count_tie_mask_[i] = true; 
				// }
				p_prev = p;
			}
		}
		std::array<ord2_t, O2> label_map = {0};
		for (ord2_t i = 0; i < O2; i++) {
			label_map[canon2orig_label[i].orig] = i;
		}
		for (auto& row : grid_) {
			for (auto& e : row) {
				e = label_map[e];
		}	}
		decltype(rel_counts_) canon_counts;
		for (ord2_t i = 0; i < O2; i++) {
			for (ord2_t j = 0; j < O2; j++) {
				canon_counts[label_map[i]][label_map[j]] = rel_count_[i][j];
		}	}
		rel_count_ = canon_counts;
		*/
	}

	#define SOLVENT_TEMPL_TEMPL(O_) \
		template grid_vec_t<O_> canon_label<O_>(grid_const_span_t<O_>);
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}