#include "solvent/grid.hpp"
#include "solvent/size.hpp"
#include "solvent/morph/rel_info.hpp"

#include <iostream> // TODO.wait remove after done implementing
#include <algorithm> // sort
#include <compare>   // weak_ordering
#include <cassert>

namespace solvent::morph {

	template<Order O>
	requires (is_order_compiled(O))
	class CanonPlace final {
		using val_t = size<O>::ord2i_least_t;
		using ord1i_t = size<O>::ord1i_t;
		using ord2i_t = size<O>::ord2i_t;
		using ord4i_t = size<O>::ord4i_t;

	public:
		static constexpr ord1i_t O1 = O;
		static constexpr ord2i_t O2 = O*O;
		static constexpr ord4i_t O3 = O*O*O;
		static constexpr ord4i_t O4 = O*O*O*O;

		struct LineSortEntry final {
			ord1i_t orig_blkline;
			std::array<std::array<val_t, O1>, O1> ramp;

			static LineSortEntry build(
				const ord1i_t orig_blkline,
				const std::span<const val_t, O2> src
			) {
				std::array<std::array<val_t, O1>, O1> ramp;
				for (ord1i_t i {0}; i < O1; ++i) {
					std::partial_sort_copy(
						std::next(src.begin(), O1*i), std::next(src.begin(), O1*(i+1)),
						ramp[i].begin(), ramp[i].end(),
						std::less{}
					);
				}
				std::ranges::sort(ramp, std::less{});
				// TODO.high turns out that the above quite easily fails for order 3... maybe can solve via passes like canon_label
				return LineSortEntry{ orig_blkline, ramp };
			}
			[[gnu::pure]] std::weak_ordering operator<=>(const LineSortEntry& that) const {
				return ramp <=> that.ramp;
			}
		};
		struct ChuteSortEntry final {
			ord1i_t orig_chute;
			std::array<LineSortEntry, O1> lines_;
			static ChuteSortEntry build(
				const ord1i_t orig_chute,
				const std::span<const val_t, O3> grid_chute
			) {
				std::array<LineSortEntry, O1> lines;
				for (ord1i_t i {0}; i < O1; ++i) { lines[i] = LineSortEntry::build(
					i, static_cast<std::span<const val_t, O2>>(grid_chute.subspan(O2*i, O2)) // *sad cast noises
				); }
				std::ranges::sort(lines, std::less());
				return ChuteSortEntry{ orig_chute, lines };
			}
			[[gnu::pure]] const LineSortEntry& operator[](size_t i) const { return lines_[i]; }
			[[gnu::pure]] std::weak_ordering operator<=>(const ChuteSortEntry& that) const {
				return lines_ <=> that.lines_;
			}
		};
		struct GridSortEntry final {
			std::array<ChuteSortEntry, O> chutes_;
			static GridSortEntry build(const grid_const_span_t<O> grid) {
				std::array<ChuteSortEntry, O> chutes;
				for (ord1i_t i {0}; i < O1; ++i) { chutes[i] = ChuteSortEntry::build(
					i, static_cast<std::span<const val_t, O3>>(grid.subspan(O3*i, O3)) // *sad cast noises
				); }
				std::sort(chutes, std::less{});
				return GridSortEntry { chutes };
			}
			[[gnu::pure]] const ChuteSortEntry& operator[](size_t i) const { return chutes_[i]; }
			[[gnu::pure]] std::weak_ordering operator<=>(const GridSortEntry& that) const {
				return chutes_ <=> that.chutes_;
			}
		};


		static void do_it(const grid_span_t<O> grid_) {
			const GridSortEntry h_grid_slide = GridSortEntry::build(grid_);
			const GridSortEntry v_grid_slide = [&](){
				std::array<val_t, O4> transposed_input;
				for (ord2i_t i {0}; i < O2; ++i) {
				for (ord2i_t j {0}; j < O2; ++j) {
					transposed_input[O2*i+j] = grid_[O2*j+i];
				}}
				return GridSortEntry::build(transposed_input);
			}();

			std::array<val_t, O4> canon_grid {O2};
			for (ord2i_t canon_row {0}; canon_row < O2; ++canon_row) {
				const auto& r_chute = h_grid_slide[canon_row/O1];
				const ord2i_t orig_row = static_cast<ord2i_t>((O1*r_chute.orig_chute) + r_chute[canon_row%O1].orig_blkline);
				for (ord2i_t canon_col {0}; canon_col < O2; ++canon_col) {
					const auto& c_chute = v_grid_slide[canon_col/O1];
					const ord2i_t orig_col = static_cast<ord2i_t>((O1*c_chute.orig_chute) + c_chute[canon_col%O1].orig_blkline);
					canon_grid[O2*canon_row+canon_col] = grid_[O2*orig_row+orig_col];
				}
			}
			if (std::is_gt(v_grid_slide <=> h_grid_slide)) {
				for (ord2i_t i {0}; i < O2; ++i) {
				for (ord2i_t j {0}; j < O2; ++j) {
					grid_[O2*i+j] = canon_grid[O2*j+i];
				}}
			} else {
				for (ord2i_t i {0}; i < O2; ++i) {
				for (ord2i_t j {0}; j < O2; ++j) {
					grid_[O2*i+j] = canon_grid[O2*i+j];
			}	}}
			(void)grid_; // TODO.high
			assert(is_sudoku_valid<O>(grid_));
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