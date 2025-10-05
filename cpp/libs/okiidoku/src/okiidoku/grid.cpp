// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/grid.hpp>

#include <okiidoku/o2_bit_arr.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <algorithm> // none_of, all_of
#include <array>
#include <execution> // execution::unseq

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
			if (col == 0u) [[unlikely]] { row_has = has_mask_t{}; }
			if (row % T::O1 == 0u) [[unlikely]] { h_chute_boxes_has.fill(has_mask_t{}); }

			const auto sym_ {grid[row,col]}; sym_.check();
			if (sym_ == T::O2) { continue; }
			const o2x_t sym {sym_};

			auto& col_has {cols_has[col]};
			auto& box_has {h_chute_boxes_has[col / T::O1]};

			if (has_mask_t::test_any3(sym, row_has, col_has, box_has)) [[unlikely]] {
				return false;
			}
			has_mask_t::set3(sym, row_has, col_has, box_has);
		}}
		return true;
	}


	template<Order O> requires(is_order_compiled(O))
	bool grid_is_filled(const Grid<O>& grid) noexcept {
		using T = Ints<O>;
		return std::all_of(
			OKIIDOKU_UNSEQ
			grid.get_underlying_array().cbegin(),
			grid.get_underlying_array().cend(),
			[][[gnu::const]](const auto sym)noexcept{ sym.check(); return sym < T::O2; }
		);
	}


	template<Order O> requires(is_order_compiled(O))
	bool grid_is_empty(const Grid<O>& grid) noexcept {
		using T = Ints<O>;
		return std::all_of(
			OKIIDOKU_UNSEQ
			grid.get_underlying_array().begin(),
			grid.get_underlying_array().end(),
			[][[gnu::const]](const auto sym)noexcept{ sym.check(); return sym == T::O2; }
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
				const auto sym_row {(boxrow+v_chute) % T::O1};
				const auto sym_col {(boxcol+h_chute) % T::O1};
				const auto sym {(T::O1 * sym_row) + sym_col};
				OKIIDOKU_CONTRACT(sym < T::O2);
				grid[rmi] = sym;
			}
		}
		OKIIDOKU_ASSERT(grid_is_filled(grid));
		OKIIDOKU_ASSERT(grid_follows_rule(grid));
	}



	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template bool grid_follows_rule<(O_)>(const Grid<(O_)>&) noexcept; \
		template bool grid_is_filled<(O_)>(const Grid<(O_)>&) noexcept; \
		template bool grid_is_empty<(O_)>(const Grid<(O_)>&) noexcept; \
		template void init_most_canonical_grid<(O_)>(Grid<(O_)>&) noexcept;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}


namespace okiidoku::visitor {

	bool grid_follows_rule(const Grid& vis_grid) noexcept {
		switch (vis_grid.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return mono::grid_follows_rule(vis_grid.unchecked_get_mono_exact<(O_)>());
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	bool grid_is_filled(const Grid& vis_grid) noexcept {
		switch (vis_grid.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return mono::grid_is_filled(vis_grid.unchecked_get_mono_exact<(O_)>());
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	bool grid_is_empty(const Grid& vis_grid) noexcept {
		switch (vis_grid.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return mono::grid_is_empty(vis_grid.unchecked_get_mono_exact<(O_)>());
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	void init_most_canonical_grid(Grid& vis_grid) noexcept {
		switch (vis_grid.get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return mono::init_most_canonical_grid(vis_grid.unchecked_get_mono_exact<(O_)>());
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}


	Grid::sym_t Grid::operator[](const ints::o4x_t rmi) const noexcept {
		switch (this->get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return this->unchecked_get_mono_exact<(O_)>()[rmi];
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	Grid::sym_t Grid::operator[](const ints::o2x_t row, const ints::o2x_t col) const noexcept {
		switch (this->get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return this->unchecked_get_mono_exact<(O_)>()[row, col];
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}
}