#include <okiidoku/grid.hpp>

#include <ranges>
#include <algorithm>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	void copy_grid(const GridConstSpan<O> src, const GridSpan<O> dest) noexcept {
		std::copy(src.cells_.begin(), src.cells_.end(), dest.cells_.begin());
	}


	template<Order O> requires(is_order_compiled(O))
	bool grid_follows_rule(const GridConstSpan<O> grid) noexcept {
		using T = traits<O>;
		using o2i_t = T::o2i_t;
		using has_mask_t = T::o2_bits_fast;

		std::array<has_mask_t, T::O2> rows_has_ {};
		std::array<has_mask_t, T::O2> cols_has_ {};
		std::array<has_mask_t, T::O2> boxes_has_ {};

		for (o2i_t row {0}; row < T::O2; ++row) {
		for (o2i_t col {0}; col < T::O2; ++col) {
			const auto val {grid.at(row,col)};
			assert(val <= T::O2);
			if (val == T::O2) { continue; }

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


	template<Order O> requires(is_order_compiled(O))
	bool grid_is_filled(const GridConstSpan<O> grid) noexcept {
		using T = traits<O>;
		using o4i_t = T::o4i_t;
		for (o4i_t i {0}; i < T::O4; ++i) {
			auto val = grid[i];
			assert(val <= T::O2);
			if (val == T::O2) { return false; }
		}
		return true;
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template void copy_grid<O_>(GridConstSpan<O_>, GridSpan<O_>) noexcept; \
		template bool grid_follows_rule<O_>(GridConstSpan<O_>) noexcept; \
		template bool grid_is_filled<O_>(GridConstSpan<O_>) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor {

	// template<class G, std::enable_if_t<is_const || (std::is_const_v<G> == is_const), bool> = true>
	// requires std::is_same_v<std::decay_t<G>, GridArr>
	// GridSpan::GridSpan(G& arr) noexcept: order_() {
	// 	;
	// }
}