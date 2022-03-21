#include "solvent/grid.hpp"
#include "solvent/morph/rel_prob.hpp"

#include <iostream> // TODO.wait remove
#include <algorithm> // swap, sort, ranges::next_permutation
#include <numeric>   // iota, abs
#include <compare>   // partial_ordering, is_eq, etc.
#include <cassert>

namespace solvent::morph {

	template<Order O>
	requires (is_order_compiled(O))
	class CanonLabel final {
		using has_mask_t = size<O>::O2_mask_least_t;
		using val_t = size<O>::ord2i_least_t;
		using ord1i_t = size<O>::ord1i_t;
		using ord2x_t = size<O>::ord2x_t;
		using ord2i_t = size<O>::ord2i_t;
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
			grid_span2d_t<O, const val_t> grid(grid_span);
			grid_arr_t<O, RelMask> masks {};
			for (ord2i_t line {0}; line < O2; ++line) {
			for (ord2i_t atom {0}; atom < O2; atom += O1) {
				// Go through all unique pairs in the atom:
				for (ord1i_t i {0}; i < O1 - 1; ++i) {
				for (ord1i_t j = i + 1; j < O1; ++j) {
					{ // boxrow
						const val_t i_val = grid.at(line, atom+i), j_val = grid.at(line, atom+j);
						const has_mask_t blk_mask_bit = has_mask_t{1} << rmi_to_blk<O>(line, atom);
						masks[i_val][j_val].blocks_h |= blk_mask_bit;
						masks[j_val][i_val].blocks_h |= blk_mask_bit;
					}
					{ // boxcol
						const val_t i_val = grid.at(atom+i, line), j_val = grid.at(atom+j, line);
						const has_mask_t blk_mask_bit = has_mask_t{1} << rmi_to_blk<O>(atom, line);
						masks[i_val][j_val].blocks_v |= blk_mask_bit;
						masks[j_val][i_val].blocks_v |= blk_mask_bit;
					}
			}}	}}
			return masks;
		}


		// Info is placement-independent.
		struct RelPlaceless final {
			double all_p;
			float polar_a_p;
			float polar_b_p;
			ord2i_t all_chute_a_occ;
			ord2i_t all_chute_b_occ;

			std::partial_ordering operator<=>(const RelPlaceless& that) const {
				std::partial_ordering cmp = all_p <=> that.all_p;
				#define M_TRY_ELSE(field) \
				if (cmp != std::partial_ordering::equivalent) [[likely]] { return cmp; } \
				cmp = field <=> that.field;
				// comments show percentage ties unbroken after try for O = 3.
				/* 50.0% */ M_TRY_ELSE(polar_a_p)
				/* 5.00% */ M_TRY_ELSE(polar_b_p)
				/* 1.80% */ M_TRY_ELSE(all_chute_a_occ) // TODO.try this might be okay to take out for O>3? needs much more testing. in which case, use conditional_t for the struct fields
				/* 1.25% */ M_TRY_ELSE(all_chute_b_occ)
				/* 0.95% */ return cmp;
			}
		};
		struct SortIngredients final {
			grid_arr_t<O, RelPlaceless> cmp_layers;
			grid_arr_t<O, double> prob_table;

			std::partial_ordering cmp_labels(const ord2x_t a_orig_label, const ord2x_t b_orig_label) const {
				for (ord2i_t layer {0}; layer < O2; ++layer) {
					const RelPlaceless& a = cmp_layers[layer][a_orig_label];
					const RelPlaceless& b = cmp_layers[layer][b_orig_label];
					const auto cmp = a <=> b;
					if (cmp != std::partial_ordering::equivalent) {
						return cmp;
					}
				}
				return std::partial_ordering::equivalent;
			};
		};
		static SortIngredients make_rel_cmp_layers_(const grid_arr_t<O, RelMask>& masks) noexcept {
			grid_arr_t<O, RelPlaceless> table;
			for (ord2i_t r {0}; r < O2; ++r) { for (ord2i_t c {0}; c < O2; ++c) {
				const RelMask& mask = masks[r][c];
				RelPlaceless& rel = table[r][c];
				const has_mask_t non_polar_mask = mask.blocks_h | mask.blocks_v;
				const int all_count = static_cast<int>((non_polar_mask.count()));
				rel.all_p     = RelCountProb<O>::all[all_count];
				rel.polar_a_p = static_cast<float>(RelCountProb<O>::polar[(mask.blocks_h.count())]);
				rel.polar_b_p = static_cast<float>(RelCountProb<O>::polar[(mask.blocks_v.count())]);

				std::array<int8_t, O1> all_chute_a_occ, all_chute_b_occ;
				for (ord1i_t chute {0}; chute < O1; ++chute) {
					all_chute_a_occ[chute] = static_cast<int8_t>((chute_blk_masks<O>::row[chute] & non_polar_mask).count());
					all_chute_b_occ[chute] = static_cast<int8_t>((chute_blk_masks<O>::col[chute] & non_polar_mask).count());
				}
				std::ranges::sort(all_chute_a_occ, std::less{});
				std::ranges::sort(all_chute_b_occ, std::less{});
				rel.all_chute_a_occ = 0;
				rel.all_chute_b_occ = 0;
				for (ord1i_t i {0}; i < O1; ++i) {
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
				ord6i_t h_occ_dev {0}, v_occ_dev {0};
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
			
			SortIngredients sort_ingredients;
			for (ord2i_t i {0}; i < O2; ++i) {
			for (ord2i_t j {0}; j < O2; ++j) {
				sort_ingredients.prob_table[i][j] = table[i][j].all_p;
			}}
			// sort, accumulate/ramp, transpose (for caching optimization)
			for (auto& row : table) {
				// put rare things at the back:
				std::ranges::sort(row, std::greater{});
			}
			for (auto& row : table) {
				for (ord2i_t i = O2-1; i > 0; i--) {
					// Iterating forward progressively removes non-rare items:
					row[i-1].all_p     *= row[i].all_p;
					row[i-1].polar_a_p *= row[i].polar_a_p;
					row[i-1].polar_b_p *= row[i].polar_b_p;
			}	}
			// TODO.try do more benchmarking comparing perf with or without transpose.
			//   some rough benchmarking on O=4 seems to indicate some benefit.
			for (ord2i_t i {0}; i < O2; ++i) {
			for (ord2i_t j {0}; j < O2; ++j) {
				sort_ingredients.cmp_layers[i][j] = table[j][i];
			}}
			return sort_ingredients;
		}


		static void find_and_break_ties_(
			const SortIngredients& sort_ingredients,
			std::array<ord2x_t, O2>& canon_to_orig
		) noexcept {
			struct Range {
				ord2x_t begin;
				ord2i_t end;
				ord2i_t size() const noexcept { return end - begin; };
			};
			std::vector<Range> tied_ranges; // entries are ranges of partially-canonical labels.

			using tieless_mask_t = size<O>::O2_mask_fast_t;
			tieless_mask_t tieless {0}; // bit places correspond to partially-canonical labels.

			for (ord2i_t tie_begin {0}, canon_label {1}; canon_label <= O2; ++canon_label) {
				if (canon_label == O2 || sort_ingredients.cmp_labels(canon_to_orig[tie_begin], canon_to_orig[canon_label]) != std::partial_ordering::equivalent) {
					if (canon_label == tie_begin + 1) [[likely]] {
						tieless |= tieless_mask_t{1} << tie_begin;
					} else {
						tied_ranges.push_back({static_cast<ord2x_t>(tie_begin), canon_label});
					}
					tie_begin = canon_label;
				}
			}
			if (tied_ranges.empty()) [[likely]] {
				return;
			}
			if (tieless.count() == 0) {
				// TODO.high encountered the most canonical grid. :O not sure what to do here.
				assert(tied_ranges.size() == 1);
				assert(tied_ranges[0].size() == O2);
				return;
			}
			for (const Range& tied_range : tied_ranges) {
				struct BreakerProbs final {
					ord2x_t partial_canon_label {};
					std::vector<double> og_probs {}; // indexed by partially-canonical labels.
				};
				std::vector<BreakerProbs> tieless_prob_rows(tied_range.size());
				for (ord2i_t i {0}; i < tied_range.size(); ++i) {
					tieless_prob_rows[i].partial_canon_label = static_cast<ord2x_t>(i + tied_range.begin);
					tieless_prob_rows[i].og_probs.resize(tieless.count());
				}
				for (ord2i_t canon_label {0}, tieless_i {0}; canon_label < O2; ++canon_label) {
					if ((tieless & (tieless_mask_t{1} << canon_label)).any()) {
						for (ord2i_t tied_label {tied_range.begin}; tied_label < tied_range.end; ++tied_label) {
							tieless_prob_rows[tied_label - tied_range.begin].og_probs[tieless_i] = sort_ingredients.prob_table[canon_to_orig[tied_label]][canon_to_orig[canon_label]];
						}
						++tieless_i;
				}	}
				std::sort(tieless_prob_rows.begin(), tieless_prob_rows.end(), [](const auto& a, const auto& b){
					return std::ranges::lexicographical_compare(a.og_probs, b.og_probs);
				});
				for (ord2i_t i {tied_range.begin}; i < tied_range.end; ++i) {
					tieless |= tieless_mask_t{1} << i; // mark tie as resolved.
					canon_to_orig[tieless_prob_rows[i - tied_range.begin].partial_canon_label] = static_cast<ord2x_t>(i);
				}
			}
		}


		//
		static void do_it(const grid_span_t<O> grid) {
			const std::array<ord2x_t, O2> label_orig_to_canon = [](const SortIngredients sort_ingredients){
				std::array<ord2x_t, O2> canon_to_orig;
				std::iota(canon_to_orig.begin(), canon_to_orig.end(), 0);
				std::sort(canon_to_orig.begin(), canon_to_orig.end(), [&](auto a, auto b){
					return sort_ingredients.cmp_labels(a, b) == std::partial_ordering::less;
				});
				find_and_break_ties_(sort_ingredients, canon_to_orig);
				std::array<ord2x_t, O2> _;
				for (ord2x_t i_canon {0}; i_canon < O2; ++i_canon) {
					_[canon_to_orig[i_canon]] = i_canon;
				}
				return _;
			}(make_rel_cmp_layers_(make_rel_masks_(grid)));

			for (ord4i_t i {0}; i < O4; ++i) {
				grid[i] = static_cast<val_t>(label_orig_to_canon[grid[i]]);
			}
			assert(is_sudoku_valid<O>(grid));
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