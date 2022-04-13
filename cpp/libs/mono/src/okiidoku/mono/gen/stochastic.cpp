#include <okiidoku/mono/gen/stochastic.hpp>

#include <algorithm> // swap, copy, shuffle

namespace okiidoku::mono::gen::ss {

	// unsigned long long total = 0;
	// unsigned long long true_ = 0;


	template<Order O>
	void Generator<O>::operator()(SharedRng& shared_rng) {
		{
			std::lock_guard lock_guard {shared_rng.mutex};
			rng_.seed(shared_rng.rng());
			for (auto& row : cells_) {
				std::shuffle(row.begin(), row.end(), shared_rng.rng);
			}
			// TODO.try should the shuffle just use `rng_`? The data-parallel implementation would be much better that way.
		}
		this->generate_();
	}


	template<Order O>
	void Generator<O>::write_to(grid_span_t<O> sink) const {
		for (o4i_t i {0}; i < T::O4; ++i) { sink[i] = cells_[i/T::O2][i%T::O2]; }
	}


	template<Order O>
	void Generator<O>::generate_() {
		/* Note: wherever you see `% .../\* -1 *\/`, that's a place where the algorithm
		would still work if it wasn't commented out, but commeting it out makes it slower
		because sometimes what would be excluded would have a faster path to validity. */
		using chute_has_counts_t = std::array<std::array<typename T::o2i_smol_t, T::O2>, T::O1>;
		// unsigned long long op_count = 0;
		/* Using this counter, I found that it took fewer operations to go from having
		one polarity of lines valid to also having boxes valid than from having only
		boxes valid and then getting one polarity of lines to also be valid. Ie. It
		should be less optimal to start only with valid boxes (and then get both cols
		and rows valid). */

		// Make boxes valid:
		for (o2i_t h_chute {0}; h_chute < T::O2; h_chute += T::O1) {
			chute_has_counts_t boxes_has {{0}};
			for (o2i_t row {h_chute}; row < h_chute+T::O1; ++row) {
			for (o2i_t col {0}; col < T::O2; ++col) {
				++(boxes_has[col/T::O1][cells_[row][col]]);
			}}
			int has_nots {0};
			for (const auto& box_has : boxes_has) {
				for (const auto& val_count : box_has) {
					if (val_count == 0) { ++has_nots; }
			}	}
			while (has_nots != 0) [[likely]] {
				const auto a_col {static_cast<o2x_t>((rng_() - rng_.min()) % T::O2)};
				const auto b_col {static_cast<o2x_t>((rng_() - rng_.min()) % T::O2)};
				const auto a_box {static_cast<o2x_t>(a_col/T::O1)};
				const auto b_box {static_cast<o2x_t>(b_col/T::O1)};
				if (a_box == b_box) [[unlikely]] { continue; }
				const auto row {static_cast<o2x_t>(h_chute + ((rng_() - rng_.min()) % (T::O1/* -1 */)))};
				auto& a_cell = cells_[row][a_col];
				auto& b_cell = cells_[row][b_col];
				const int has_nots_diff {
					(boxes_has[a_box][a_cell] == 1 ?  1 : 0) +
					(boxes_has[a_box][b_cell] == 0 ? -1 : 0) +
					(boxes_has[b_box][b_cell] == 1 ?  1 : 0) +
					(boxes_has[b_box][a_cell] == 0 ? -1 : 0)
				};
				if (has_nots_diff <= 0) [[unlikely]] { // TODO.low for fun: find out on average at what op_count it starts being unlikely
					has_nots += has_nots_diff;
					--boxes_has[a_box][a_cell];
					++boxes_has[a_box][b_cell];
					--boxes_has[b_box][b_cell];
					++boxes_has[b_box][a_cell];
					std::swap(a_cell, b_cell);
				}
				// ++op_count;
			}
		}
		// std::cout << "\n" << op_count << ", ";
		// op_count = 0;

		// Make columns valid:
		for (o2i_t v_chute {0}; v_chute < T::O2; v_chute += T::O1) {
			chute_has_counts_t cols_has {{0}};
			for (o2i_t row {0}; row < T::O2; ++row) {
			for (o2i_t box_col {0}; box_col < T::O1; ++box_col) {
				++(cols_has[box_col][cells_[row][v_chute+box_col]]);
			}}
			int has_nots {0};
			for (const auto& col_has : cols_has) {
				for (const auto& val_count : col_has) {
					if (val_count == 0) { ++has_nots; }
			}	}
			while (has_nots != 0) [[likely]] {
				const auto a_col {static_cast<o2x_t>((rng_() - rng_.min()) % T::O1)};
				const auto b_col {static_cast<o2x_t>((rng_() - rng_.min()) % T::O1)};
				if (a_col == b_col) [[unlikely]] { continue; }
				const auto row {static_cast<o2x_t>((rng_() - rng_.min()) % (T::O1*(T::O1/* -1 */)))};
				auto& a_cell = cells_[row][v_chute + a_col];
				auto& b_cell = cells_[row][v_chute + b_col];
				const int has_nots_diff {
					(cols_has[a_col][a_cell] == 1 ?  1 : 0) +
					(cols_has[a_col][b_cell] == 0 ? -1 : 0) +
					(cols_has[b_col][b_cell] == 1 ?  1 : 0) +
					(cols_has[b_col][a_cell] == 0 ? -1 : 0)
				};
				if (has_nots_diff <= 0) [[unlikely]] {
					has_nots += has_nots_diff;
					--cols_has[a_col][a_cell];
					++cols_has[a_col][b_cell];
					--cols_has[b_col][b_cell];
					++cols_has[b_col][a_cell];
					std::swap(a_cell, b_cell);
				}
				// ++op_count;
			}
		}
		// std::cout << op_count;

		#ifndef NDEBUG
		grid_arr_flat_t<O> grid;
		this->write_to(std::span(grid));
		assert(grid_follows_rule<O>(std::span(grid)));
		#endif
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class Generator<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}