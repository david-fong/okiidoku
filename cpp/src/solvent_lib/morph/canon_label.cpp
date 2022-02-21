#include <solvent_lib/morph/analyze.hpp>
#include <solvent_lib/print.hpp> // TODO remove after done implementing
#include <solvent_lib/morph/rel_prob.hpp>

#include <iostream>
#include <algorithm> // sort
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

		struct RelCountPass1Entry {
			has_mask_t blocks_h;
			has_mask_t blocks_v;
		};
		grid_mtx_t<O, RelCountPass1Entry<O>> rel_counts_pass1_(std::span<const ord4_t, O4> grid) noexcept {
			grid_mtx_t<O, RelCountPass1Entry<O>> table;
			for (ord2_t line = 0; line < O2; line++) {
				for (ord2_t atom = 0; atom < O2; atom += O1) {
					// Go through all unique pairs in the atom:
					for (ord1_t i = 0; i < O1 - 1; i++) {
						for (ord1_t j = i + 1; j < O1; j++) {
							{ // boxrow
								const ord2_t i_val = grid[line][atom+i], j_val = grid[line][atom+j];
								const has_mask_t blk_mask_bit = 1 << rmi2blk<O>(line, atom);
								rel_count_[i_val][j_val].blocks_h |= blk_mask_bit;
								rel_count_[j_val][i_val].blocks_h |= blk_mask_bit;
							}
							{ // boxcol
								const ord2_t i_val = grid[atom+i][line], j_val = grid[atom+j][line];
								const has_mask_t blk_mask_bit = 1 << rmi2blk<O>(atom, line);
								rel_count_[i_val][j_val].blocks_v |= blk_mask_bit;
								rel_count_[j_val][i_val].blocks_v |= blk_mask_bit;
							}
			}	}	}	}
			return table;
		}

		struct RelCountPass2Entry {
			;
		};
		template<Order O>
		grid_mtx_t<O, RelCountPass2Entry<O>> rel_counts_pass2_(const grid_mtx_t<O, RelCountPass1Entry<O>>& pass1_table) noexcept {
			grid_mtx_t<O, RelCountPass2Entry<O>> table;
			for (auto& row : rel_count_) { for (auto& rel : row) {
				std::array<ord2_t, (2*O)-1> density_counts = {0};
				const has_mask_t blocks_mask = rel.blocks_h | rel.blocks_v;
				rel.count = 0;
				for (ord1_t r = 0; r < O; r++) {
					for (ord1_t c = 0; c < O; c++) {
						const has_mask_t blk_mask = blk_mask_chutes.row[r] & blk_mask_chutes.col[c];
						if (blk_mask & blocks_mask) {
							rel.count++;
							const has_mask_t chute_see_mask = blk_mask_chutes.row[r] | blk_mask_chutes.col[c];
							density_counts[std::popcount(blocks_mask & chute_see_mask)-1]++;
						}
				}	}
				rel.density = 0;
				for (unsigned i = 0; i < density_counts.size(); i++) {
					rel.density += density_counts[i] * std::pow(O2, i);
				}
			}}
			return table;
		}
	};

	template<Order O>
	grid_vec_t<O> canon_label(const grid_vec_t<O>& input) {
		const auto rel_info1 = res_counts_pass1_(input);
		const auto rel_info2 = res_counts_pass1_(rel_info1);
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
		template grid_vec_t<O_> canon_label<O_>(const grid_vec_t<O_>&);
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}