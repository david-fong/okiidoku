#include <okiidoku/morph/transform.hpp>

#include <algorithm> // copy

namespace okiidoku::mono::morph {

	template<Order O>
	void Transformation<O>::apply_from_to(const Grid<O>& src_grid, Grid<O>& dest_grid) const noexcept {
		for (o2i_t src_row {0}; src_row < T::O2; ++src_row) {
		for (o2i_t src_col {0}; src_col < T::O2; ++src_col) {
			auto dest_row = row_map[src_row/T::O1][src_row%T::O1];
			auto dest_col = col_map[src_col/T::O1][src_col%T::O1];
			if (transpose) { std::swap(dest_row, dest_col); }
			const auto src_label {src_grid.at(src_row, src_col)};
			dest_grid.at(dest_row, dest_col) = (src_label == T::O2) ? T::O2 : label_map[src_label];
		}}
	}


	template<Order O>
	void Transformation<O>::apply_in_place(Grid<O>& grid) const noexcept {
		const Grid<O> copy {grid};
		apply_from_to(copy, grid);
	}


	template<Order O>
	Transformation<O> Transformation<O>::inverted() const noexcept {
		Transformation<O> _;
		for (o2i_t i {0}; i < T::O2; ++i) {
			_.label_map[label_map[i]] = static_cast<mapping_t>(i);
		}
		for (o2i_t i {0}; i < T::O2; ++i) {
			const auto row_inv = row_map[i/T::O1][i%T::O1];
			const auto col_inv = col_map[i/T::O1][i%T::O1];
			_.row_map[row_inv/T::O1][row_inv%T::O1] = static_cast<mapping_t>(i);
			_.col_map[col_inv/T::O1][col_inv%T::O1] = static_cast<mapping_t>(i);
		}
		_.transpose = transpose;
		// assert(this->operator==(_.inverted()));
		// TODO.low do the assert without causing infinite recursion or move it to tests.cpp
		return _;
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template struct Transformation<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}


namespace okiidoku::visitor::morph {

	void Transformation::apply_from_to(const Grid& vis_src, Grid& vis_dest) const noexcept {
		if (this->get_mono_order() != vis_src.get_mono_order()) {
			return;
		}
		if (vis_src.get_mono_order() != vis_dest.get_mono_order()) {
			vis_dest.get_mono_variant() = vis_src.get_mono_variant(); // lazy-coding way to implement order change.
		}
		return std::visit([&]<Order O1, Order O2, Order O3>( // TODO.asap
			const mono::morph::Transformation<O1>& mono_transform,
			const mono::Grid<O2>& mono_src,
			mono::Grid<O3>& mono_dest
		) {
			if constexpr (O1 == O2 && O2 == O3) { return mono_transform.apply_from_to(mono_src, mono_dest); }
		}, this->get_mono_variant(), vis_src.get_mono_variant(), vis_dest.get_mono_variant());
	}


	void Transformation::apply_in_place(Grid& vis_grid) const noexcept {
		if (this->get_mono_order() != vis_grid.get_mono_order()) {
			return;
		}
		return std::visit([&]<Order O1, Order O2>( // TODO.asap
			const mono::morph::Transformation<O1>& mono_transform,
			mono::Grid<O2>& mono_grid
		) {
			if constexpr (O1 == O2) { return mono_transform.apply_in_place(mono_grid); }
		}, this->get_mono_variant(), vis_grid.get_mono_variant());
	}


	Transformation Transformation::inverted() const noexcept {
		return std::visit([&](const auto& mono_transform) {
			return static_cast<Transformation>(mono_transform.inverted());
		}, this->get_mono_variant());
	}
}