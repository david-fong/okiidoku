#include <solvent_lib/equiv/scramble.hpp>

#include <algorithm>   // shuffle,

namespace solvent::lib::equiv {

	std::mt19937 ScramblerRng;

	template<Order O>
	grid_vec_t<O> scramble(grid_vec_t<O> const& input_vec) {
		using ord1_t = typename size<O>::ord1_t;
		using ord2_t = typename size<O>::ord2_t;
		using ord4_t = typename size<O>::ord4_t;
		static constexpr ord1_t O1 = O;
		static constexpr ord2_t O2 = O*O;
		static constexpr ord4_t O4 = O*O*O*O;
		std::array<std::array<ord2_t, O2>, O2> input;
		for (ord2_t row = 0; row < O2; row++) {
			for (ord2_t col = 0; col < O2; col++) {
				input[row][col] = input_vec[(O2*row)+col];
			}
		}

		std::array<ord2_t, O2> label_map;
		std::array<std::array<ord2_t, O1>, O1> row_map;
		std::array<std::array<ord2_t, O1>, O1> col_map;
		bool transpose;

		for (ord2_t i = 0; i < O2; i++) {
			label_map[i] = i;
			row_map[i/O1][i%O1] = i;
			col_map[i/O1][i%O1] = i;
		}
		std::ranges::shuffle(label_map, ScramblerRng);
		std::ranges::shuffle(row_map, ScramblerRng);
		std::ranges::shuffle(col_map, ScramblerRng);
		for (ord1_t chute = 0; chute < O1; chute++) {
			std::ranges::shuffle(row_map[chute], ScramblerRng);
			std::ranges::shuffle(col_map[chute], ScramblerRng);
		}
		transpose = ScramblerRng() % 2;
		
		grid_vec_t<O> output_vec(O4);
		for (ord2_t row = 0; row < O2; row++) {
			for (ord2_t col = 0; col < O2; col++) {
				ord2_t mapped_row = row_map[row/O1][row%O1];
				ord2_t mapped_col = col_map[col/O1][col%O1];
				if (transpose) { std::swap(mapped_row, mapped_col); }
				output_vec[(O2*row)+col] = label_map[input[mapped_row][mapped_col]];
			}
		}
		return output_vec;
	}

	#define SOLVENT_TEMPL_TEMPL(O_) \
		template vec_grid_t<O_> scramble<O_>(vec_grid_t<O_> const&); \
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}