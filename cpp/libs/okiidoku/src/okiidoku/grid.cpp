#include <okiidoku/grid.hpp>

#include <ranges>
#include <algorithm>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	bool grid_follows_rule(const Grid<O>& grid) noexcept {
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
	bool grid_is_filled(const Grid<O>& grid) noexcept {
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
		template bool grid_follows_rule<O_>(const Grid<O_>&) noexcept; \
		template bool grid_is_filled<O_>(const Grid<O_>&) noexcept;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor {

	bool grid_follows_rule(const Grid& vis_grid) noexcept {
		return std::visit([&](auto& mono_grid){
			return mono::grid_follows_rule(mono_grid);
		}, vis_grid.get_mono_variant());
	}

	bool grid_is_filled(const Grid& vis_grid) noexcept {
		return std::visit([&](auto& mono_grid){
			return mono::grid_is_filled(mono_grid);
		}, vis_grid.get_mono_variant());
	}

	std::strong_ordering operator<=>(const Grid& vis_a, const Grid& vis_b) noexcept {
		if (const auto cmp {vis_a.get_mono_order() <=> vis_b.get_mono_order()}; std::is_neq(cmp)) {
			return cmp;
		}
		switch (vis_a.get_mono_order()) {
		#define OKIIDOKU_FOR_COMPILED_O(O_) \
		case O_: return vis_a.get_mono_exact<O_>() <=> vis_b.get_mono_exact<O_>();
		OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef OKIIDOKU_FOR_COMPILED_O
		}
		return std::strong_ordering::equivalent; // TODO.wait std::unreachable
	}


	Grid::common_val_t Grid::at_row_major(const traits::o4i_t coord) const noexcept {
		return std::visit([&](const auto& mono_this){
			return common_val_t{mono_this.at_row_major(coord)};
		}, get_mono_variant());
	}

	Grid::common_val_t Grid::at(const traits::o2i_t row, const traits::o2i_t col) const noexcept {
		return std::visit([&](const auto& mono_this){
			return common_val_t{mono_this.at(row, col)};
		}, get_mono_variant());
	}
}