#include <ookiidoku/morph/transform.hpp>

#include <algorithm> // copy

namespace ookiidoku::morph {

	template<Order O>
	void Transformation<O>::apply_to(const grid_span_t<O> grid) const noexcept {
		std::array<typename size<O>::ord2i_least_t, O4> orig_grid;
		std::copy(grid.begin(), grid.end(), orig_grid.begin());

		for (ord2i_t og_row {0}; og_row < O2; ++og_row) {
		for (ord2i_t og_col {0}; og_col < O2; ++og_col) {
			ord2i_t mapped_row = row_map[og_row/O1][og_row%O1];
			ord2i_t mapped_col = col_map[og_col/O1][og_col%O1];
			if (transpose) { std::swap(mapped_row, mapped_col); }
			auto orig_label = orig_grid[(O2*og_row)+og_col];
			grid[(O2*mapped_row) + mapped_col] = (orig_label == O2) ? O2 : label_map[orig_label];
		}}
	}


	#define M_OOKIIDOKU_TEMPL_TEMPL(O_) \
		template struct Transformation<O_>;
	M_OOKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OOKIIDOKU_TEMPL_TEMPL
}