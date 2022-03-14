#include "solvent/print.hpp" // TODO remove after done implementing
#include "solvent/grid.hpp"
#include "solvent/size.hpp"
#include "solvent/morph/rel_prob.hpp"
#include "solvent_export.h"

#include <iostream>
#include <algorithm> // sort
#include <compare>   // partial_ordering
#include <cassert>

namespace solvent::morph {

	template<Order O>
	requires ((O > 0) && (O <= O_MAX))
	class SOLVENT_NO_EXPORT /* <- just in case. */ CanonPlace final {
		using has_mask_t = size<O>::O2_mask_least_t;
		using ord1i_t = size<O>::ord1i_t;
		using ord2i_t = size<O>::ord2i_t;
		using ord4i_t = size<O>::ord4i_t;

	public:
		static constexpr ord1i_t O1 = O;
		static constexpr ord2i_t O2 = O*O;
		static constexpr unsigned O3 = O*O*O;
		static constexpr ord4i_t O4 = O*O*O*O;
		// [[gnu::pure]] ord2i_t operator[](ord4i_t coord) const override;

		struct LineSortEntry final {
			ord1i_t orig_blkline;
			double prob_polar;
			static LineSortEntry build(const grid_arr_t<O, ord2i_t>& counts, ord1i_t orig_blkline, const std::span<const ord2i_t, O2> line) {
				double prob_polar = 1.0;
				for (ord2i_t atom {0}; atom < O2; atom += O1) {
					for (ord1i_t i {0}; i < O1-1; ++i) {
						for (ord1i_t j = i+1; j < O1; ++j) {
							prob_polar *= RelCountProb<O>::all[counts[line[atom+i]][line[atom+j]]];
				}	}	}
				return LineSortEntry { .orig_blkline = orig_blkline, .prob_polar = prob_polar };
			}
			[[gnu::pure]] std::partial_ordering operator<=>(const LineSortEntry& that) const {
				return prob_polar <=> that.prob_polar;
			}
		};
		struct ChuteSortEntry final {
			ord1i_t orig_chute;
			double prob_all;
			double prob_polar;
			std::array<LineSortEntry, O> lines_;
			static ChuteSortEntry build(const grid_arr_t<O, ord2i_t>& counts, ord1i_t orig_chute, const std::span<const ord2i_t, O3> grid_chute) {
				std::array<LineSortEntry, O> lines;
				for (ord1i_t i {0}; i < O1; ++i) { lines[i] = LineSortEntry::build(
					counts, i, static_cast<std::span<const ord2i_t, O2>>(grid_chute.subspan(O2*orig_chute, O2)) // *sad cast noises
				); }
				std::sort(lines.begin(), lines.end());
				double prob_polar = 1.0; for (const auto& e : lines) { prob_polar *= e.prob_polar; }
				double prob_all = prob_polar; // TODO
				return ChuteSortEntry { .orig_chute = orig_chute, .prob_all = prob_all, .prob_polar = prob_polar, .lines_ = lines };
			}
			[[gnu::pure]] const LineSortEntry& operator[](ord1i_t i) const { return lines_[i]; }
			[[gnu::pure]] std::partial_ordering operator<=>(const ChuteSortEntry& that) const {
				return prob_all <=> that.prob_all;
			}
		};
		struct GridSortEntry final {
			double prob;
			std::array<ChuteSortEntry, O> chutes_;
			static GridSortEntry build(const grid_arr_t<O, ord2i_t>& counts, const std::span<const ord2i_t, O4> grid) {
				std::array<ChuteSortEntry, O> chutes;
				for (ord1i_t i {0}; i < O1; ++i) { chutes[i] = ChuteSortEntry::build(
					counts, i, static_cast<std::span<const ord2i_t, O3>>(grid.subspan(O3*i, O3)) // *sad cast noises
				); }
				std::sort(chutes.begin(), chutes.end());
				double prob = 1.0; for (const auto& e : chutes) { prob *= e.prob_polar; }
				return GridSortEntry { .prob = prob, .chutes_ = chutes };
			}
			[[gnu::pure]] const ChuteSortEntry& operator[](ord1i_t i) const { return chutes_[i]; }
			[[gnu::pure]] std::partial_ordering operator<=>(const GridSortEntry& that) const {
				return prob <=> that.prob;
			}
		};


		static void do_it(const grid_span_t<O> grid) {
			/* const GridSortEntry grid_slide = GridSortEntry::build(rel_count_, grid_);
			const GridSortEntry transposed_grid_slide = [this](){
				decltype(grid_) transposed_input;
				for (ord2i_t i {0}; i < O2; ++i) {
					for (ord2i_t j {0}; j < O2; ++j) {
						transposed_input[i][j] = grid_[j][i];
					}
				}
				return GridSortEntry::build(rel_count_, transposed_input);
			}();

			decltype(grid_) canon_input {O2};
			for (ord2i_t canon_row {0}; canon_row < O2; ++canon_row) {
				const auto& r_chute = grid_slide[canon_row/O1];
				const ord2i_t orig_row = (O1*r_chute.orig_chute) + r_chute[canon_row%O1].orig_blkline;
				for (ord2i_t canon_col {0}; canon_col < O2; ++canon_col) {
					const auto& c_chute = transposed_grid_slide[canon_col/O1];
					const ord2i_t orig_col = (O1*c_chute.orig_chute) + c_chute[canon_col%O1].orig_blkline;
					canon_input[canon_row][canon_col] = grid_[orig_row][orig_col];
				}
			}
			if (transposed_grid_slide < grid_slide) {
				for (ord2i_t i {0}; i < O2; ++i) {
					for (ord2i_t j {0}; j < O2; ++j) {
						grid_[i][j] = canon_input[j][i];
					}
				}
			} else {
				grid_ = canon_input;
			} */
			(void)grid; // TODO
			assert(is_sudoku_valid<O>(grid));
		}
	};


	template<Order O>
	void canon_place(const grid_span_t<O> grid) {
		return CanonPlace<O>::do_it(grid);
	}


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template void canon_place<O_>(grid_span_t<O_>); \
		template class CanonPlace<O_>;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}