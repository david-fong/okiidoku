#include <okiidoku/grid.hpp>

#include <algorithm>

namespace okiidoku::mono {

	template<Order O, class V_>
	GridArr<O, V_> GridArr<O, V_>::copy_from_span(GridConstSpan<O, typename GridArr<O, V_>::V> src) noexcept {
		GridArr _;
		std::ranges::copy(src.cells_, _.cells_);
		return _;
	}


	template<Order O>
	requires(is_order_compiled(O))
	bool grid_follows_rule(const GridConstSpan<O> grid) noexcept {
		using o2i_t = typename traits<O>::o2i_t;
		using has_mask_t = typename traits<O>::o2_bits_fast;
		static constexpr o2i_t O2 {O*O};

		std::array<has_mask_t, O2> rows_has_ {};
		std::array<has_mask_t, O2> cols_has_ {};
		std::array<has_mask_t, O2> boxes_has_ {};

		for (o2i_t row {0}; row < O2; ++row) {
		for (o2i_t col {0}; col < O2; ++col) {
			const auto val {grid.at(row,col)};
			assert(val <= O2);
			if (val == O2) { continue; }

			auto& row_has = rows_has_[row];
			auto& col_has = cols_has_[col];
			auto& box_has = boxes_has_[rmi_to_box<O>(row, col)];

			const has_mask_t try_val_mask {has_mask_t{1} << val};
			const has_mask_t t_has {row_has | col_has | box_has};
			if ((t_has & try_val_mask).any()) [[unlikely]] {
				return false;
			} else {
				row_has |= try_val_mask;
				col_has |= try_val_mask;
				box_has |= try_val_mask;
			}
		}}
		return true;
	}


	template<Order O>
	requires(is_order_compiled(O))
	bool grid_is_filled(const GridConstSpan<O> grid) noexcept {
		using o4i_t = traits<O>::o4i_t;
		for (o4i_t i {0}; i < traits<O>::O4; ++i) {
			auto val = grid[i];
			assert(val <= traits<O>::O2);
			if (val == traits<O>::O2) { return false; }
		}
		return true;
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template bool grid_follows_rule<O_>(GridConstSpan<O_>) noexcept; \
		template bool grid_is_filled<O_>(GridConstSpan<O_>) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}