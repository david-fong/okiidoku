#include <ookiidoku/morph/transform.hpp>

#include <algorithm> // copy

namespace ookiidoku::morph {

	template<Order O>
	void Transformation<O>::apply_from_to(const grid_const_span_t<O> src_grid, const grid_span_t<O> dest_grid) const noexcept {
		for (ord2i_t src_row {0}; src_row < O2; ++src_row) {
		for (ord2i_t src_col {0}; src_col < O2; ++src_col) {
			auto dest_row = row_map[src_row/O1][src_row%O1];
			auto dest_col = col_map[src_col/O1][src_col%O1];
			if (transpose) { std::swap(dest_row, dest_col); }
			const auto src_label = src_grid[(O2*src_row)+src_col];
			dest_grid[(O2*dest_row) + dest_col] = (src_label == O2) ? O2 : label_map[src_label];
		}}
	}


	template<Order O>
	void Transformation<O>::apply_in_place(const grid_span_t<O> grid) const noexcept {
		grid_arr_flat_t<O> og_grid;
		std::copy(grid.begin(), grid.end(), og_grid.begin());
		apply_from_to(og_grid, grid);
	}


	template<Order O>
	Transformation<O> Transformation<O>::inverted() const noexcept {
		Transformation<O> _;
		for (ord2i_t i {0}; i < O2; ++i) {
			_.label_map[label_map[i]] = static_cast<mapping_t>(i);
		}
		for (ord2i_t i {0}; i < O2; ++i) {
			const auto row_inv = row_map[i/O1][i%O1];
			const auto col_inv = col_map[i/O1][i%O1];
			_.row_map[row_inv/O1][row_inv%O1] = static_cast<mapping_t>(i);
			_.col_map[col_inv/O1][col_inv%O1] = static_cast<mapping_t>(i);
		}
		_.transpose = transpose;
		assert(_.inverted() == *this);
		return _;
	}


	#define M_OOKIIDOKU_TEMPL_TEMPL(O_) \
		template struct Transformation<O_>;
	M_OOKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef M_OOKIIDOKU_TEMPL_TEMPL
}