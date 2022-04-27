#include <okiidoku/morph/transform.hpp>

#include <algorithm> // copy

namespace okiidoku::mono::morph {

	template<Order O>
	void Transformation<O>::apply_from_to(const GridConstSpan<O> src_grid, const GridSpan<O> dest_grid) const noexcept {
		for (o2i_t src_row {0}; src_row < T::O2; ++src_row) {
		for (o2i_t src_col {0}; src_col < T::O2; ++src_col) {
			auto dest_row = row_map[src_row/T::O1][src_row%T::O1];
			auto dest_col = col_map[src_col/T::O1][src_col%T::O1];
			if (transpose) { std::swap(dest_row, dest_col); }
			const auto src_label = src_grid.at(src_row, src_col);
			dest_grid.at(dest_row, dest_col) = (src_label == T::O2) ? T::O2 : label_map[src_label];
		}}
	}


	template<Order O>
	void Transformation<O>::apply_in_place(const GridSpan<O> grid) const noexcept {
		GridArr<O> og_grid;
		copy_grid<O>(grid, og_grid);
		apply_from_to(og_grid, grid);
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

	void Transformation::apply_from_to(const GridConstSpan visitor_src_grid, const GridSpan visitor_dest_grid) const noexcept {
		return std::visit([&]<Order O_transform, Order O_src, Order O_dest>(
			const mono::morph::Transformation<O_transform>& mono_transform,
			const mono::GridConstSpan<O_src>& mono_src_grid,
			const mono::GridSpan<O_dest>& mono_dest_grid
		) {
			if constexpr (O_transform == O_src && O_transform == O_dest) {
				return mono_transform.apply_from_to(mono_src_grid, mono_dest_grid);
			}
		}, this->get_mono_variant(), visitor_src_grid.get_mono_variant(), visitor_dest_grid.get_mono_variant());
	}


	void Transformation::apply_in_place(const GridSpan visitor_grid) const noexcept {
		return std::visit([&]<Order O_transform, Order O_grid>(
			const mono::morph::Transformation<O_transform>& mono_transform,
			const mono::GridSpan<O_grid>& mono_grid
		) {
			if constexpr (O_transform == O_grid) {
				return mono_transform.apply_in_place(mono_grid);
			}
		}, this->get_mono_variant(), visitor_grid.get_mono_variant());
	}


	Transformation Transformation::inverted() const noexcept {
		return std::visit([&](const auto& mono_transform) {
			return static_cast<Transformation>(mono_transform.inverted());
		}, this->get_mono_variant());
	}
}