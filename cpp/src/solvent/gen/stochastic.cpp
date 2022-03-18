#include "solvent/gen/stochastic.hpp"
#include "solvent/rng.hpp"

#include <algorithm> // swap, shuffle

namespace solvent::gen::ss {

	// unsigned long long total = 0;
	// unsigned long long true_ = 0;


	std::unique_ptr<Generator> Generator::create(const Order order) {
		switch (order) {
			#define M_SOLVENT_TEMPL_TEMPL(O_) \
			case O_: return std::make_unique<gen::ss::GeneratorO<O_>>();
			M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
			#undef M_SOLVENT_TEMPL_TEMPL

			default: return std::make_unique<gen::ss::GeneratorO<M_SOLVENT_DEFAULT_ORDER>>();
		}
	}


	template<Order O>
	void GeneratorO<O>::operator()() {
		{
			std::lock_guard lock_guard {shared_mt_rng_mutex_};
			rng_.seed(shared_mt_rng_());
			for (auto& row : cells_) {
				std::shuffle(row.begin(), row.end(), shared_mt_rng_);
			}
			// TODO.try should the shuffle just use `rng_`? The data-parallel implementation would be much better that way.
		}
		this->generate_();
	}


	template<Order O>
	GeneratorO<O>::val_t GeneratorO<O>::get_val_at_(const GeneratorO<O>::ord4x_t coord) const noexcept {
		return cells_[coord/O2][coord%O2];
	}


	template<Order O>
	void GeneratorO<O>::generate_() {
		/* Note: wherever you see `% .../\* -1 *\/`, that's a place where the algorithm
		would still work if it wasn't commented out, but commeting it out makes it slower
		because sometimes what would be excluded would have a faster path to validity. */
		using ord2i_least_t = size<O>::ord2i_least_t;
		using chute_has_counts_t = std::array<std::array<ord2i_least_t, O2>, O1>;
		// unsigned long long op_count = 0;
		/* Using this counter, I found that it took fewer operations to go from having
		one polarity of lines valid to also having blocks valid than from having only
		blocks valid and then getting one polarity of lines to also be valid. Ie. It
		should be less optimal to start only with valid blocks (and then get both cols
		and rows valid). */

		// Make blocks valid:
		for (ord2i_t h_chute {0}; h_chute < O2; h_chute += O1) {
			chute_has_counts_t blks_has {{0}};
			for (ord2i_t row {h_chute}; row < h_chute+O1; ++row) {
				for (ord2i_t col {0}; col < O2; ++col) {
					++(blks_has[col/O1][cells_[row][col]]);
			}	}
			int has_nots {0};
			for (const auto& blk_has : blks_has) {
				for (const auto& val_count : blk_has) {
					if (val_count == 0) { ++has_nots; }
			}	}
			while (has_nots != 0) [[likely]] {
				const ord2x_t a_col {static_cast<ord2x_t>((rng_() - rng_.min()) % O2)};
				const ord2x_t b_col {static_cast<ord2x_t>((rng_() - rng_.min()) % O2)};
				const ord2x_t a_blk {static_cast<ord2x_t>(a_col/O1)};
				const ord2x_t b_blk {static_cast<ord2x_t>(b_col/O1)};
				if (a_blk == b_blk) [[unlikely]] { continue; }
				const ord2x_t row {static_cast<ord2x_t>(h_chute + ((rng_() - rng_.min()) % (O1/* -1 */)))};
				auto& a_cell = cells_[row][a_col];
				auto& b_cell = cells_[row][b_col];
				const int has_nots_diff {
					(blks_has[a_blk][a_cell] == 1 ?  1 : 0) +
					(blks_has[a_blk][b_cell] == 0 ? -1 : 0) +
					(blks_has[b_blk][b_cell] == 1 ?  1 : 0) +
					(blks_has[b_blk][a_cell] == 0 ? -1 : 0)
				};
				if (has_nots_diff <= 0) [[unlikely]] { // TODO.low for fun: find out on average at what op_count it starts being unlikely
					has_nots += has_nots_diff;
					--blks_has[a_blk][a_cell];
					++blks_has[a_blk][b_cell];
					--blks_has[b_blk][b_cell];
					++blks_has[b_blk][a_cell];
					std::swap(a_cell, b_cell);
				}
				// ++op_count;
			}
		}
		// std::cout << "\n" << op_count << ", ";
		// op_count = 0;

		// Make columns valid:
		for (ord2i_t v_chute {0}; v_chute < O2; v_chute += O1) {
			chute_has_counts_t cols_has {{0}};
			for (ord2i_t row {0}; row < O2; ++row) {
				for (ord2i_t blk_col {0}; blk_col < O1; ++blk_col) {
					++(cols_has[blk_col][cells_[row][v_chute+blk_col]]);
			}	}
			int has_nots {0};
			for (const auto& col_has : cols_has) {
				for (const auto& val_count : col_has) {
					if (val_count == 0) { ++has_nots; }
			}	}
			while (has_nots != 0) [[likely]] {
				const ord2x_t a_col {static_cast<ord2x_t>((rng_() - rng_.min()) % O1)};
				const ord2x_t b_col {static_cast<ord2x_t>((rng_() - rng_.min()) % O1)};
				if (a_col == b_col) [[unlikely]] { continue; }
				const ord2x_t row {static_cast<ord2x_t>((rng_() - rng_.min()) % (O1*(O1/* -1 */)))};
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
		std::array<ord2i_least_t, O4> grid;
		this->write_to_(std::span(grid));
		assert(is_sudoku_valid<O>(std::span<const ord2i_least_t, O4>(grid)));
		#endif
	}


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template class GeneratorO<O_>;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}