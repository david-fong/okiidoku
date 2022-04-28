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
			auto val = grid.at_row_major(i);
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

	bool grid_follows_rule(const GridConstSpan grid) noexcept {
		return std::visit([&](auto& mono_grid){
			return mono::grid_follows_rule(mono_grid);
		}, grid.get_mono_variant());
	}

	bool grid_is_filled(const GridConstSpan grid) noexcept {
		return std::visit([&](auto& mono_grid){
			return mono::grid_is_filled(mono_grid);
		}, grid.get_mono_variant());
	}

	GridArr::common_val_t GridArr::at_row_major(const traits::o4i_t coord) const noexcept {
		return std::visit([&](auto& mono_this){
			return common_val_t{mono_this.at_row_major(coord)};
		}, get_mono_variant());
	}

	GridArr::common_val_t GridArr::at(const traits::o2i_t row, const traits::o2i_t col) const noexcept {
		return std::visit([&](auto& mono_this){
			return common_val_t{mono_this.at(row, col)};
		}, get_mono_variant());
	}

	namespace detail {
		GridSpan<true>::GridSpan(const GridSpan<false>& other) noexcept: GridSpan(static_cast<const GridSpan<true>&>(other)) {}

		#define OKIIDOKU_FOR_COMPILED_O(O_) \
			template<> GridSpan<true>::GridSpan<O_>(const mono::GridSpan<O_> mono_span) noexcept: GridSpan<true>(static_cast<mono::GridConstSpan<O_>>(mono_span)) {}
		OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef OKIIDOKU_FOR_COMPILED_O

		GridSpan<false>::GridSpan(GridArr& arr) noexcept: GridSpan(std::visit(
			[&]<Order O>(mono::GridArr<O>& mono_arr){
				return variant_t(mono::GridSpan<O>(mono_arr));
			},
			arr.get_mono_variant()
		)) {}

		GridSpan<true>::GridSpan(const GridArr& arr) noexcept: GridSpan(std::visit(
			[&]<Order O>(const mono::GridArr<O>& mono_arr){
				return variant_t(mono::GridConstSpan<O>(mono_arr));
			},
			arr.get_mono_variant()
		)) {}

		template<bool is_const>
		GridSpan<is_const>::common_val_t GridSpan<is_const>::at_row_major(const traits::o4i_t coord) const noexcept {
			return std::visit([&](auto& mono_this){
				return common_val_t{mono_this.at_row_major(coord)};
			}, this->get_mono_variant());
		}

		template<bool is_const>
		GridSpan<is_const>::common_val_t GridSpan<is_const>::at(const traits::o2i_t row, const traits::o2i_t col) const noexcept {
			return std::visit([&](auto& mono_this){
				return common_val_t{mono_this.at(row, col)};
			}, this->get_mono_variant());
		}


		template class GridSpan<false>;
		template class GridSpan<true>;
	}
}