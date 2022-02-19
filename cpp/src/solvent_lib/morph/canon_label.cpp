#include <solvent_lib/morph/analyze.hpp>
#include <solvent_lib/print.hpp> // TODO remove after done implementing
#include <solvent_util/math.hpp>

#include <iostream>
#include <algorithm> // sort
#include <numeric>   // transform_reduce
#include <execution> // execution::par_unseq
#include <cmath>     // pow
#include <compare>   // partial_ordering
#include <bit>       // popcount

namespace solvent::lib::morph {

	template<Order O>
	class GridInnerRelStats final {

	 public:
		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;
		// [[gnu::pure]] ord2_t operator[](ord4_t coord) const override;

		GridInnerRelStats(const grid_vec_t<O>&);
		grid_vec_t<O> operator()(void);

	 private:
		void analyze_rel_counts_(void) noexcept;


		/** The probability of values A and B being in the same atom within a
		block is `p(n) = 2/(o+1)` (simplified from `2(o-1)/(o^2-1)`). The
		probability of this ocurring k times in a grid is given by a binomial
		distribution B(o^2, 2/(o+1)). */
		static constexpr std::array<double, O2+1> REL_COUNT_ALL_PROB = [](){
			std::array<double, O2+1> _;
			for (unsigned i = 0; i < O2+1; i++) {
				_[i] = static_cast<double>(n_choose_r(O2, i) * (1<<i) * std::pow(O1-1, O2-i)) / std::pow(O1+1, O2); }
			return _;
		}();

		static constexpr std::array<double, O2+1> REL_COUNT_POLAR_PROB = [](){
			std::array<double, O2+1> _;
			for (unsigned i = 0; i < O2+1; i++) {
				_[i] = static_cast<double>(n_choose_r(O2, i) * std::pow(O1, O2-i)) / std::pow(O1+1, O2); }
			return _;
		}();
	};


	template<Order O>
	void GridInnerRelStats<O>::analyze_rel_counts_(void) noexcept {
		// Initialize rel_count_:
		for (ord2_t line = 0; line < O2; line++) {
			for (ord2_t atom = 0; atom < O2; atom += O1) {
				// Go through all unique pairs in the atom:
				for (ord1_t atom_i = 0; atom_i < O1 - 1; atom_i++) {
					for (ord1_t atom_j = atom_i + 1; atom_j < O1; atom_j++) {
						{ // boxrow
							const ord2_t label_i = grid_[line][atom+atom_i], label_j = grid_[line][atom+atom_j];
							const has_mask_t blk_mask_bit = 1 << rmi2blk<O>(line, atom);
							rel_count_[label_i][label_j].blocks_h |= blk_mask_bit;
							rel_count_[label_j][label_i].blocks_h |= blk_mask_bit;
						}
						{ // boxcol
							const ord2_t label_i = grid_[atom+atom_i][line], label_j = grid_[atom+atom_j][line];
							const has_mask_t blk_mask_bit = 1 << rmi2blk<O>(atom, line);
							rel_count_[label_i][label_j].blocks_v |= blk_mask_bit;
							rel_count_[label_j][label_i].blocks_v |= blk_mask_bit;
						}
		}	}	}	}
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
			ord1_t area_y = 0, area_x = 0;
			for (ord1_t i = 0; i < O; i++) { area_y += (blk_mask_chutes.row[i] & blocks_mask) != 0; }
			for (ord1_t i = 0; i < O; i++) { area_x += (blk_mask_chutes.col[i] & blocks_mask) != 0; }
			rel.area = area_x * area_y;
		}}
/*
		// Set up Floyd-Warshall:
		grid_mtx_t<O, double> dist_graph;
		for (ord2_t i = 0; i < O2; i++) {
			for (ord2_t j = 0; j < O2; j++) {
				dist_graph[i][j] = std::pow(O2, O2 - rel_count_[i][j].all);
		}	}
		for (ord2_t i = 0; i < O2; i++) { dist_graph[i][i] = 0.0; }

		// Do Floyd-Warshall:
		for (ord2_t k = 0; k < O2; k++) {
			for (ord2_t i = 0; i < O2; i++) {
				for (ord2_t j = 0; j < O2; j++) {
					double other = dist_graph[i][k] + dist_graph[k][j];
					double& it = dist_graph[i][j];
					if (it > other) { it = other; }
		}	}	}

		// Sort vertices by Closeness-Centrality (reciprocal of dist):
		for (ord2_t label = 0; label < O2; label++) {
			const auto& paths = dist_graph[label];
			const double dist = std::accumulate(paths.cbegin(), paths.cend(), 0.0);
			const double p_all = std::transform_reduce(
				std::execution::par_unseq, rel_count_[label].cbegin(), rel_count_[label].cend(),
				1.0, std::multiplies<double>(), [](const auto& rel) { return REL_COUNT_ALL_PROB[rel.all]; }
			);
			canon2orig_label[label] = SortMapEntry {
				.orig = label, .dist = dist, .p_all = p_all,
			};
		} */
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

}