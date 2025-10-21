// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/grid.hpp>

#include <okiidoku/bit_array.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>
#include <okiidoku/detail/util.hpp>

#include <algorithm> // none_of, all_of
#include <array>
#include <execution> // execution::unseq

namespace okiidoku::mono {

	template<Order O> requires(is_order_compiled(O))
	bool Grid<O>::follows_rule() const noexcept {
		using has_mask_t = O2BitArr<O>;

		has_mask_t row_has;
		std::array<has_mask_t, T::O1> h_chute_boxes_has;
		std::array<has_mask_t, T::O2> cols_has;

		for (const auto row : T::O2) {
		for (const auto col : T::O2) {
			if (col == 0u) [[unlikely]] { row_has = has_mask_t{}; }
			if (row % T::O1 == 0u) [[unlikely]] { h_chute_boxes_has.fill(has_mask_t{}); }

			const auto sym_ {operator[](row,col)}; sym_.check();
			if (sym_ == T::O2) { continue; }
			const o2x_t sym {*sym_};

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
	bool Grid<O>::is_filled() const noexcept {
		return std::all_of(OKIIDOKU_UNSEQ
			B::arr.cbegin(), B::arr.cend(),
			[][[gnu::const]](const auto sym)noexcept{ sym.check(); return sym < T::O2; }
		);
	}


	template<Order O> requires(is_order_compiled(O))
	bool Grid<O>::is_empty() const noexcept {
		return std::all_of(OKIIDOKU_UNSEQ
			B::arr.begin(), B::arr.end(),
			[][[gnu::const]](const auto sym)noexcept{ sym.check(); return sym == T::O2; }
		);
	}


	// TODO.low compare binary size and speed if this is changed to initialize from a constexpr variable
	template<Order O> requires(is_order_compiled(O))
	void Grid<O>::init_most_canonical() noexcept {
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
				operator[](rmi) = sym;
			}
		}
		OKIIDOKU_ASSERT(is_filled());
		OKIIDOKU_ASSERT(follows_rule());
	}



	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		template struct Grid<(O_)>;
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}


namespace okiidoku::visitor {

	bool Grid::follows_rule() const noexcept {
		switch (this->get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return this->unchecked_get_mono_exact<(O_)>().follows_rule();
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	bool Grid::is_filled() const noexcept {
		switch (this->get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return this->unchecked_get_mono_exact<(O_)>().is_filled();
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	bool Grid::is_empty() const noexcept {
		switch (this->get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return this->unchecked_get_mono_exact<(O_)>().is_empty();
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		default: OKIIDOKU_UNREACHABLE;
		}
	}

	void Grid::init_most_canonical() noexcept {
		switch (this->get_order()) {
		#define OKIIDOKU_FOREACH_O_EMIT(O_) \
		case (O_): return this->unchecked_get_mono_exact<(O_)>().init_most_canonical();
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