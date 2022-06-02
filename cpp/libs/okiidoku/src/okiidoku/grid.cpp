#include <okiidoku/grid.hpp>

#include <okiidoku/o2_bit_arr.hpp>

#include <algorithm>
#include <execution>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	bool grid_follows_rule(const Grid<O>& grid) noexcept {
		using T = Ints<O>;
		using o2x_t = int_ts::o2x_t<O>;
		using o2i_t = int_ts::o2i_t<O>;
		using has_mask_t = O2BitArr<O>;

		has_mask_t row_has;
		std::array<has_mask_t, T::O1> h_chute_boxes_has;
		std::array<has_mask_t, T::O2> cols_has;

		for (o2i_t row {0}; row < T::O2; ++row) {
		for (o2i_t col {0}; col < T::O2; ++col) {
			if (col == 0) [[unlikely]] { row_has = has_mask_t{}; }
			if (row % T::O1 == 0) [[unlikely]] { h_chute_boxes_has.fill(has_mask_t{}); }

			const auto& val {grid.at(row,col)};
			assert(val <= T::O2);
			if (val == T::O2) { continue; }

			auto& col_has {cols_has[col]};
			auto& box_has {h_chute_boxes_has[col / T::O1]};

			if (has_mask_t::test_any3(static_cast<o2x_t>(val), row_has, col_has, box_has)) [[unlikely]] {
				return false;
			} else {
				has_mask_t::set3(static_cast<o2x_t>(val), row_has, col_has, box_has);
			}
		}}
		return true;
	}


	template<Order O> requires(is_order_compiled(O))
	bool grid_is_filled(const Grid<O>& grid) noexcept {
		using T = Ints<O>;
		using o4i_t = int_ts::o4i_t<O>;
		for (o4i_t i {0}; i < T::O4; ++i) {
			const auto& val {grid.at_rmi(i)};
			assert(val <= T::O2);
			if (val == T::O2) { return false; }
		}
		return true;
	}


	template<Order O> requires(is_order_compiled(O))
	bool grid_is_empty(const Grid<O>& grid) noexcept {
		using T = Ints<O>;
		return std::all_of(
			std::execution::par_unseq,
			grid.get_underlying_array().cbegin(),
			grid.get_underlying_array().cend(),
			[](const auto val){ return val == T::O2; }
		);
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template bool grid_follows_rule<O_>(const Grid<O_>&) noexcept; \
		template bool grid_is_filled<O_>(const Grid<O_>&) noexcept; \
		template bool grid_is_empty<O_>(const Grid<O_>&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor {

	bool grid_follows_rule(const Grid& vis_grid) noexcept {
		return std::visit([](auto& mono_grid){
			return mono::grid_follows_rule(mono_grid);
		}, vis_grid.get_mono_variant());
	}

	bool grid_is_filled(const Grid& vis_grid) noexcept {
		return std::visit([](auto& mono_grid){
			return mono::grid_is_filled(mono_grid);
		}, vis_grid.get_mono_variant());
	}

	bool grid_is_empty(const Grid& vis_grid) noexcept {
		return std::visit([](auto& mono_grid){
			return mono::grid_is_empty(mono_grid);
		}, vis_grid.get_mono_variant());
	}


	Grid::common_val_t Grid::at_rmi(const int_ts::o4i_t rmi) const noexcept {
		return std::visit([&](const auto& mono_this){
			return common_val_t{mono_this.at_rmi(rmi)};
		}, get_mono_variant());
	}

	Grid::common_val_t Grid::at(const int_ts::o2i_t row, const int_ts::o2i_t col) const noexcept {
		return std::visit([&](const auto& mono_this){
			return common_val_t{mono_this.at(row, col)};
		}, get_mono_variant());
	}
}