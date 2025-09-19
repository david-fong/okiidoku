// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/grid.hpp>

#include <okiidoku/o2_bit_arr.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <algorithm> // all_of
#include <array>
#include <execution>

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	bool grid_follows_rule(const Grid<O>& grid) noexcept {
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		using has_mask_t = O2BitArr<O>;

		has_mask_t row_has;
		std::array<has_mask_t, T::O1> h_chute_boxes_has;
		std::array<has_mask_t, T::O2> cols_has;

		for (const auto row : T::O2) {
		for (const auto col : T::O2) {
			if (col == 0) [[unlikely]] { row_has = has_mask_t{}; }
			if (row % T::O1 == 0) [[unlikely]] { h_chute_boxes_has.fill(has_mask_t{}); }

			const auto val_ {grid.at(row,col)}; val_.check();
			if (val_ == T::O2) { continue; }
			const o2x_t val {val_};

			auto& col_has {cols_has[col]};
			auto& box_has {h_chute_boxes_has[col / T::O1]};

			if (has_mask_t::test_any3(val, row_has, col_has, box_has)) [[unlikely]] {
				return false;
			}
			has_mask_t::set3(val, row_has, col_has, box_has);
		}}
		return true;
	}


	template<Order O> requires(is_order_compiled(O))
	bool grid_is_filled(const Grid<O>& grid) noexcept {
		using T = Ints<O>;
		return std::none_of(
			#ifdef __cpp_lib_execution
			std::execution::unseq,
			#endif
			grid.get_underlying_array().cbegin(),
			grid.get_underlying_array().cend(),
			[](const auto& val){ return val == T::O2; }
		);
	}


	template<Order O> requires(is_order_compiled(O))
	bool grid_is_empty(const Grid<O>& grid) noexcept {
		using T = Ints<O>;
		return std::all_of(
			#ifdef __cpp_lib_execution
			std::execution::unseq,
			#endif
			grid.get_underlying_array().cbegin(),
			grid.get_underlying_array().cend(),
			[](const auto val) noexcept { return val == T::O2; }
		);
	}


	// TODO.low compare binary size and speed if this is changed to initialize from a constexpr variable
	template<Order O> requires(is_order_compiled(O))
	void init_most_canonical_grid(Grid<O>& grid) noexcept {
		OKIIDOKU_MONO_INT_TS_TYPEDEFS
		for (const auto box : T::O2) {
			const auto h_chute {box/T::O1};
			const auto v_chute {box%T::O1};
			for (const auto box_cell : T::O2) {
				const auto boxrow {box_cell/T::O1};
				const auto boxcol {box_cell%T::O1};
				const auto rmi {box_cell_to_rmi<O>(box, box_cell)};
				const auto val_row {(boxrow+v_chute) % T::O1};
				const auto val_col {(boxcol+h_chute) % T::O1};
				const auto val {static_cast<grid_val_t<O>>(
					(T::O1*val_row)+val_col
				)};
				OKIIDOKU_CONTRACT_USE(val < T::O2);
				grid.at_rmi(rmi) = val;
			}
		}
		OKIIDOKU_CONTRACT_ASSERT(grid_is_filled(grid));
		OKIIDOKU_CONTRACT_ASSERT(grid_follows_rule(grid));
	}



	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template bool grid_follows_rule<O_>(const Grid<O_>&) noexcept; \
		template bool grid_is_filled<O_>(const Grid<O_>&) noexcept; \
		template bool grid_is_empty<O_>(const Grid<O_>&) noexcept; \
		template void init_most_canonical_grid<O_>(Grid<O_>&) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}


namespace okiidoku::visitor {

	bool grid_follows_rule(const Grid& vis_grid) noexcept {
		switch (vis_grid.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return mono::grid_follows_rule(vis_grid.unchecked_get_mono_exact<O_>());
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	bool grid_is_filled(const Grid& vis_grid) noexcept {
		switch (vis_grid.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return mono::grid_is_filled(vis_grid.unchecked_get_mono_exact<O_>());
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	bool grid_is_empty(const Grid& vis_grid) noexcept {
		switch (vis_grid.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return mono::grid_is_empty(vis_grid.unchecked_get_mono_exact<O_>());
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	void init_most_canonical_grid(Grid& vis_grid) noexcept {
		switch (vis_grid.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return mono::init_most_canonical_grid(vis_grid.unchecked_get_mono_exact<O_>());
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}


	Grid::val_t Grid::at_rmi(const ints::o4x_t rmi) const noexcept {
		switch (this->get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return this->unchecked_get_mono_exact<O_>().at_rmi( \
			mono::Ints<O_>::o4x_t{rmi} \
		);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	Grid::val_t Grid::at(const ints::o2x_t row, const ints::o2x_t col) const noexcept {
		switch (this->get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case O_: return this->unchecked_get_mono_exact<O_>().at( \
			mono::Ints<O_>::o2x_t{row}, \
			mono::Ints<O_>::o2x_t{col} \
		);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}
}