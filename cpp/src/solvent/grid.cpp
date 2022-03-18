#include "solvent/grid.hpp"

namespace solvent {

	template<Order O>
	bool is_sudoku_valid(const grid_const_span_t<O> grid) noexcept {
		using ord2i_t = typename size<O>::ord2i_t;
		using has_mask_t = typename size<O>::O2_mask_fast_t;
		static constexpr ord2i_t O2 {O*O};

		std::array<has_mask_t, O2> rows_has_ {};
		std::array<has_mask_t, O2> cols_has_ {};
		std::array<has_mask_t, O2> blks_has_ {};

		for (ord2i_t row {0}; row < O2; ++row) {
			for (ord2i_t col {0}; col < O2; ++col) {
				const auto val {grid[(O2*row) + col]};
				assert(val <= O2);
				if (val == O2) { continue; }

				auto& row_has = rows_has_[row];
				auto& col_has = cols_has_[col];
				auto& blk_has = blks_has_[rmi_to_blk<O>(row, col)];

				const has_mask_t try_val_mask {has_mask_t{1} << val};
				const has_mask_t t_has {row_has | col_has | blk_has};
				if ((t_has & try_val_mask).any()) [[unlikely]] {
					return false;
				} else {
					row_has |= try_val_mask;
					col_has |= try_val_mask;
					blk_has |= try_val_mask;
				}
		}	}
		return true;
	}


	template<Order O>
	bool is_sudoku_filled(const grid_const_span_t<O> grid) noexcept {
		static constexpr typename size<O>::ord2i_t O2 {O*O};
		for (auto val : grid) {
			assert(val <= O2);
			if (val >= O2) { return false; }
		}
		return true;
	}


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template bool is_sudoku_valid<O_>(grid_const_span_t<O_>) noexcept; \
		template bool is_sudoku_filled<O_>(grid_const_span_t<O_>) noexcept;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}