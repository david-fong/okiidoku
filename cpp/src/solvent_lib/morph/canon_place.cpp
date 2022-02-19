#include <solvent_lib/grid.hpp>
#include <solvent_lib/size.hpp>
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
	class CanonPlace final {
	 static_assert(O > 0 && O < MAX_REASONABLE_ORDER);
		using has_mask_t = typename size<O>::O2_mask_least_t;
		using ord1_t = typename size<O>::ord1_t;
		using ord2_t = typename size<O>::ord2_t;
		using ord4_t = typename size<O>::ord4_t;

	 public:
		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;
		// [[gnu::pure]] ord2_t operator[](ord4_t coord) const override;

		/** Carries position-independent information about the way two
		labels relate to one another in a grid. */
		struct LineSortEntry final {
			ord1_t orig_blkline;
			double prob_polar;
			static LineSortEntry build(const grid_mtx_t<O, RelStats>& counts, ord1_t orig_blkline, const std::array<ord2_t, O*O>& line);
			[[gnu::pure]] std::partial_ordering operator<=>(const LineSortEntry& that) const;
		};
		struct ChuteSortEntry final {
			ord1_t orig_chute;
			double prob_all;
			double prob_polar;
			std::array<LineSortEntry, O> lines_;
			static ChuteSortEntry build(const grid_mtx_t<O, RelStats>& counts, ord1_t orig_chute, const grid_arr_t& grid);
			[[gnu::const]] const LineSortEntry& operator[](ord1_t i) const { return lines_[i]; }
			[[gnu::const]] std::partial_ordering operator<=>(const ChuteSortEntry& that) const;
		};
		struct GridSortEntry final {
			double prob;
			std::array<ChuteSortEntry, O> chutes_;
			static GridSortEntry build(const grid_mtx_t<O, RelStats>& counts, const grid_arr_t& grid);
			[[gnu::const]] const ChuteSortEntry& operator[](ord1_t i) const { return chutes_[i]; }
			[[gnu::const]] std::partial_ordering operator<=>(const GridSortEntry& that) const;
		};

	 public:
		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;
		// [[gnu::pure]] ord2_t operator[](ord4_t coord) const override;

		CanonPlace(const grid_vec_t<O>&);
		grid_vec_t<O> operator()(void);

	 private:
		void analyze_rel_counts_(void) noexcept;

	};


	template<Order O>
	void canon_place(grid_vec_t<O>& grid) {
		// TODO assert that input is the correct length and is a complete, valid sudoku?
		CanonPlace<O> stats(grid);
		return stats();
	}

	template<Order O>
	CanonPlace<O>::CanonPlace(const grid_vec_t<O>& input):
		grid_(lib::grid_vec2mtx<O>(input))
	{
	}

	template<Order O>
	grid_vec_t<O> CanonPlace<O>::operator()(void) {
		analyze_rel_counts_();
		grid_vec_t<O> ret = grid_mtx2vec<O>(grid_);
		return ret;
	}


	template<Order O>
	CanonPlace<O>::LineSortEntry CanonPlace<O>::LineSortEntry::build(
		const grid_mtx_t<O, RelStats>& counts, const ord1_t orig_blkline, const std::array<ord2_t, O*O>& line
	) {
		double prob_polar = 1.0;
		for (ord2_t atom = 0; atom < O2; atom += O1) {
			for (ord1_t i = 0; i < O1-1; i++) {
				for (ord1_t j = i+1; j < O1; j++) {
					prob_polar *= REL_COUNT_ALL_PROB[counts[line[atom+i]][line[atom+j]].count];
		}	}	}
		return LineSortEntry { .orig_blkline = orig_blkline, .prob_polar = prob_polar };
	}
	template<Order O>
	CanonPlace<O>::ChuteSortEntry CanonPlace<O>::ChuteSortEntry::build(
		const grid_mtx_t<O, RelStats>& counts, const ord1_t orig_chute, const grid_arr_t& grid
	) {
		std::array<LineSortEntry, O> lines;
		for (ord1_t i = 0; i < O1; i++) { lines[i] = LineSortEntry::build(counts, i, grid[(O1*orig_chute)+i]); }
		std::sort(lines.begin(), lines.end());
		double prob_polar = 1.0; for (const auto& e : lines) { prob_polar *= e.prob_polar; }
		double prob_all = prob_polar; // TODO
		return ChuteSortEntry { .orig_chute = orig_chute, .prob_all = prob_all, .prob_polar = prob_polar, .lines_ = lines };
	}
	template<Order O>
	CanonPlace<O>::GridSortEntry CanonPlace<O>::GridSortEntry::build(
		const grid_mtx_t<O, RelStats>& counts, const grid_arr_t& grid
	) {
		std::array<ChuteSortEntry, O> chutes;
		for (ord1_t i = 0; i < O1; i++) { chutes[i] = ChuteSortEntry::build(counts, i, grid); }
		std::sort(chutes.begin(), chutes.end());
		double prob = 1.0; for (const auto& e : chutes) { prob *= e.prob_polar; }
		return GridSortEntry { .prob = prob, .chutes_ = chutes };
	}


	template<Order O>
	std::partial_ordering CanonPlace<O>::LineSortEntry::operator<=>(const LineSortEntry& that) const {
		return prob_polar <=> that.prob_polar;
	}
	template<Order O>
	std::partial_ordering CanonPlace<O>::ChuteSortEntry::operator<=>(const ChuteSortEntry& that) const {
		return prob_all <=> that.prob_all;
	}
	template<Order O>
	std::partial_ordering CanonPlace<O>::GridSortEntry::operator<=>(const GridSortEntry& that) const {
		return prob <=> that.prob;
	}


	
	template<Order O>
	void canonicalize_positioning_() {
		// TODO
		/* const GridSortEntry grid_slide = GridSortEntry::build(rel_count_, grid_);
		const GridSortEntry transposed_grid_slide = [this](){
			decltype(grid_) transposed_input;
			for (ord2_t i = 0; i < O2; i++) {
				for (ord2_t j = 0; j < O2; j++) {
					transposed_input[i][j] = grid_[j][i];
				}
			}
			return GridSortEntry::build(rel_count_, transposed_input);
		}();

		decltype(grid_) canon_input = {O2};
		for (ord2_t canon_row = 0; canon_row < O2; canon_row++) {
			const auto& r_chute = grid_slide[canon_row/O1];
			const ord2_t orig_row = (O1*r_chute.orig_chute) + r_chute[canon_row%O1].orig_blkline;
			for (ord2_t canon_col = 0; canon_col < O2; canon_col++) {
				const auto& c_chute = transposed_grid_slide[canon_col/O1];
				const ord2_t orig_col = (O1*c_chute.orig_chute) + c_chute[canon_col%O1].orig_blkline;
				canon_input[canon_row][canon_col] = grid_[orig_row][orig_col];
			}
		}
		if (transposed_grid_slide < grid_slide) {
			for (ord2_t i = 0; i < O2; i++) {
				for (ord2_t j = 0; j < O2; j++) {
					grid_[i][j] = canon_input[j][i];
				}
			}
		} else {
			grid_ = canon_input;
		} */
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template void canon_place<O_>(grid_vec_t<O_>&); \
		template class CanonPlace<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}