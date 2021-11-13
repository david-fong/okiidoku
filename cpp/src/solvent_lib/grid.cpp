#include <solvent_lib/grid.hpp>

namespace solvent::lib {

	template<Order O>
	grid_vec_t<O> grid_mtx2vec(const grid_mtx_t<O>& mtx) noexcept {
		grid_vec_t<O> vec;
		for (unsigned row = 0; row < O*O; row++) {
			for (unsigned col = 0; col < O*O; col++) {
				vec[(row*O*O)+col] = mtx[row][col];
			}
		}
		return vec;
	}


	template<Order O>
	grid_mtx_t<O> grid_vec2mtx(const grid_vec_t<O>& vec) noexcept {
		grid_mtx_t<O> mtx;
		for (unsigned row = 0; row < O*O; row++) {
			for (unsigned col = 0; col < O*O; col++) {
				mtx[row][col] = vec[(row*O*O)+col];
			}
		}
		return mtx;
	}


	template<Order O>
	bool is_grid_invalid(const grid_mtx_t<O>& grid) noexcept {
		using ord1_t = typename size<O>::ord1_t;
		using ord2_t = typename size<O>::ord2_t;
		using ord4_t = typename size<O>::ord4_t;
		using has_mask_t = typename size<O>::has_mask_t;
		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;

		std::array<has_mask_t, O2> rows_has_ {0};
		std::array<has_mask_t, O2> cols_has_ {0};
		std::array<has_mask_t, O2> blks_has_ {0};

		for (ord4_t coord = 0; coord < O4; coord++) {
			has_mask_t& row_has = rows_has_[rmi2row<O>(coord)];
			has_mask_t& col_has = cols_has_[rmi2col<O>(coord)];
			has_mask_t& blk_has = blks_has_[rmi2blk<O>(coord)];

			const has_mask_t t_has = (row_has | col_has | blk_has);
			const has_mask_t try_val_mask = has_mask_t(1) << grid[coord];
			if (t_has & try_val_mask) [[unlikely]] {
				return true;
			} else {
				row_has |= try_val_mask;
				col_has |= try_val_mask;
				blk_has |= try_val_mask;
			}
		}
		return false;
	}
}


template class std::vector<std::uint_fast8_t>;
#define SOLVENT_TEMPL_TEMPL(O_) \
	template class std::array<std::array<typename solvent::size<O_>::ord2_t, O_*O_>, O_*O_>;
SOLVENT_INSTANTIATE_ORDER_TEMPLATES
#undef SOLVENT_TEMPL_TEMPL