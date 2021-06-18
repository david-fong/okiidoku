#include <solvent_lib/grid.hpp>

#include <iostream>

namespace solvent::lib {

	template<Order O>
	bool AbstractGrid<O>::is_grid_valid(void) const {
		using has_mask_t = typename size<O>::has_mask_t;
		std::array<has_mask_t, O2> rows_has_ {};
		std::array<has_mask_t, O2> cols_has_ {};
		std::array<has_mask_t, O2> blks_has_ {};

		for (ord4_t coord = 0; coord < O4; coord++) {
			has_mask_t& row_has = rows_has_[this->get_row(coord)];
			has_mask_t& col_has = cols_has_[this->get_col(coord)];
			has_mask_t& blk_has = blks_has_[this->get_blk(coord)];

			const has_mask_t t_has = (row_has | col_has | blk_has);
			const has_mask_t try_val_mask = has_mask_t(1) << this->operator[](coord);
			if (t_has & try_val_mask) [[unlikely]] {
				return false;
			} else {
				row_has |= try_val_mask;
				col_has |= try_val_mask;
				blk_has |= try_val_mask;
			}
		}
		return true;
	}

	#define SOLVENT_TEMPL_TEMPL(O_) \
		template class AbstractGrid<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}