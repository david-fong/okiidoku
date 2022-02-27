#include <solvent_lib/grid.hpp>

namespace solvent::lib {

	template<Order O>
	grid_vec_t<O> grid_mtx2vec(const grid_arr_t<O>& mtx) noexcept {
		grid_vec_t<O> vec(O*O*O*O);
		for (unsigned row = 0; row < O*O; row++) {
			for (unsigned col = 0; col < O*O; col++) {
				vec[(row*O*O)+col] = mtx[row][col];
			}
		}
		return vec;
	}


	template<Order O>
	grid_arr_t<O> grid_vec2mtx(const grid_vec_t<O>& vec) noexcept {
		grid_arr_t<O> mtx;
		for (unsigned row = 0; row < O*O; row++) {
			for (unsigned col = 0; col < O*O; col++) {
				mtx[row][col] = vec[(row*O*O)+col];
			}
		}
		return mtx;
	}


	template<Order O>
	bool is_grid_valid(const grid_const_span_t<O> grid) noexcept {
		using ord2i_t = typename size<O>::ord2i_t;
		using has_mask_t = typename size<O>::O2_mask_fast_t;
		static constexpr ord2i_t O2 = O*O;

		std::array<has_mask_t, O2> rows_has_ {0};
		std::array<has_mask_t, O2> cols_has_ {0};
		std::array<has_mask_t, O2> blks_has_ {0};

		for (ord2i_t row = 0; row < O*O; row++) {
			for (ord2i_t col = 0; col < O*O; col++) {
				has_mask_t& row_has = rows_has_[row];
				has_mask_t& col_has = cols_has_[col];
				has_mask_t& blk_has = blks_has_[rmi2blk<O>(row, col)];

				const has_mask_t t_has = (row_has | col_has | blk_has);
				const has_mask_t try_val_mask = has_mask_t(1) << grid[row][col];
				if (t_has & try_val_mask) [[unlikely]] {
					return false;
				} else {
					row_has |= try_val_mask;
					col_has |= try_val_mask;
					blk_has |= try_val_mask;
				}
			}
		}
		return true;
	}
}


// template class std::vector<std::uint_fast8_t>;
#define M_SOLVENT_TEMPL_TEMPL(O_) \
	template class std::array<std::array<typename solvent::size<O_>::ord2i_t, O_*O_>, O_*O_>;
M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
#undef M_SOLVENT_TEMPL_TEMPL