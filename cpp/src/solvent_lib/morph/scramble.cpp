#include <solvent_lib/morph/scramble.hpp>

#include <array>
#include <algorithm>   // shuffle,
#include <random>

namespace solvent::lib::morph {

	std::mt19937 ScramblerRng_;
	void seed_scrambler_rng(const std::uint_fast64_t seed) noexcept {
		ScramblerRng_.seed(seed);
	}

	template<Order O>
	grid_vec_t<O> scramble(const grid_vec_t<O>& input_vec) {
		using ord1i_t = size<O>::ord1i_t;
		using ord2i_t = size<O>::ord2i_t;
		static constexpr ord1i_t O1 = O;
		static constexpr ord2i_t O2 = O*O;
		static constexpr typename size<O>::ord4i_t O4 = O*O*O*O;
		grid_mtx_t<O> input = grid_vec2mtx<O>(input_vec);

		std::array<ord2i_t, O2> label_map;
		bool transpose = false;
		std::array<std::array<ord1i_t, O1>, O1> row_map;
		std::array<std::array<ord1i_t, O1>, O1> col_map;

		for (ord2i_t i = 0; i < O2; i++) {
			label_map[i] = i;
			row_map[i/O1][i%O1] = i;
			col_map[i/O1][i%O1] = i;
		}
		std::ranges::shuffle(label_map, ScramblerRng_);
		// std::ranges::shuffle(row_map, ScramblerRng_);
		// std::ranges::shuffle(col_map, ScramblerRng_);
		// for (ord1i_t chute = 0; chute < O1; chute++) {
		// 	std::ranges::shuffle(row_map[chute], ScramblerRng_);
		// 	std::ranges::shuffle(col_map[chute], ScramblerRng_);
		// }
		// transpose = static_cast<bool>(ScramblerRng_() % 2);
		// TODO uncomment once canon_label seems to be working.
		
		grid_vec_t<O> output_vec(O4);
		for (ord2i_t row = 0; row < O2; row++) {
			for (ord2i_t col = 0; col < O2; col++) {
				ord2i_t mapped_row = row_map[row/O1][row%O1];
				ord2i_t mapped_col = col_map[col/O1][col%O1];
				if (transpose) { std::swap(mapped_row, mapped_col); }
				output_vec[(O2*mapped_row)+mapped_col] = label_map[input[row][col]];
			}
		}
		return output_vec;
	}

	#define SOLVENT_TEMPL_TEMPL(O_) \
		template grid_vec_t<O_> scramble<O_>(const grid_vec_t<O_>&);
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}