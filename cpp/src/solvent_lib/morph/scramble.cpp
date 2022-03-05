#include "solvent_lib/morph/scramble.hpp"

#include <array>
#include <algorithm>   // shuffle, ranges::copy
#include <random>
#include <cassert>

namespace solvent::lib::morph {

	static std::mt19937 ScramblerRng_;
	void seed_scrambler_rng(const std::uint_fast64_t seed) noexcept {
		ScramblerRng_.seed(seed);
	}


	template<Order O>
	void scramble(const grid_span_t<O> grid) {
		using ord1i_t = size<O>::ord1i_t;
		using ord2i_t = size<O>::ord2i_t;
		using ord2x_t = size<O>::ord2x_t;
		static constexpr ord1i_t O1 = O;
		static constexpr ord2i_t O2 = O*O;
		static constexpr typename size<O>::ord4i_t O4 = O*O*O*O;
		std::array<ord2i_t, O4> orig_grid;
		std::copy(grid.begin(), grid.end(), orig_grid.begin());

		std::array<ord2x_t, O2> label_map;
		bool transpose = false;
		std::array<std::array<ord1i_t, O1>, O1> row_map;
		std::array<std::array<ord1i_t, O1>, O1> col_map;

		for (ord2i_t i = 0; i < O2; ++i) {
			label_map[i] = i;
			row_map[i/O1][i%O1] = i;
			col_map[i/O1][i%O1] = i;
		}
		std::ranges::shuffle(label_map, ScramblerRng_);
		// std::ranges::shuffle(row_map, ScramblerRng_);
		// std::ranges::shuffle(col_map, ScramblerRng_);
		// for (ord1i_t chute = 0; chute < O1; ++chute) {
		// 	std::ranges::shuffle(row_map[chute], ScramblerRng_);
		// 	std::ranges::shuffle(col_map[chute], ScramblerRng_);
		// }
		// transpose = static_cast<bool>(ScramblerRng_() % 2);
		// TODO uncomment once canon_label seems to be working.
		
		for (ord2i_t row = 0; row < O2; ++row) {
			for (ord2i_t col = 0; col < O2; ++col) {
				ord2i_t mapped_row = row_map[row/O1][row%O1];
				ord2i_t mapped_col = col_map[col/O1][col%O1];
				if (transpose) { std::swap(mapped_row, mapped_col); }
				auto orig_label = orig_grid[(O2*row)+col];
				grid[(O2*mapped_row)+mapped_col] = orig_label == O2 ? O2 : label_map[orig_label];
			}
		}
		assert(is_grid_valid<O>(grid));
	}


	template<class T>
	requires std::is_integral_v<T>
	void scramble(Order order, std::span<T> grid) {
		assert(is_order_compiled(order));
		assert(grid.size() >= order*order*order*order);
		switch (order) {
		#define M_SOLVENT_TEMPL_TEMPL(O_) \
			case O_: { \
				constexpr unsigned O4 = O_*O_*O_*O_; \
				using val_t = size<O_>::ord2i_t; \
				std::array<val_t,O4> grid_resize; \
				for (unsigned i = 0; i < O4; ++i) { grid_resize[i] = static_cast<val_t>(grid[i]); } \
				scramble<O_>(std::span(grid_resize)); \
				for (unsigned i = 0; i < O4; ++i) { grid[i] = static_cast<T>(grid_resize[i]); } \
				break; \
			}
		M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef M_SOLVENT_TEMPL_TEMPL
		}
	}


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template void scramble<O_>(const grid_span_t<O_>);
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}