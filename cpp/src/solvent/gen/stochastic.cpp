#include "solvent/gen/stochastic.hpp"
#include "solvent/print.hpp"
#include "solvent/rng.hpp"

#include <algorithm> // swap, shuffle

#include <iostream>
namespace solvent::gen::ss {

	// long long total = 0;
	// long long true_ = 0;

	Params Params::clean(const Order O) noexcept {
		if (max_ops == 0) {
			max_ops = opcount::limit_default[O];
		} else if (max_ops > opcount::limit_i_max[O]) {
			max_ops = opcount::limit_i_max[O];
		}
		return *this;
	}


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
	void GeneratorO<O>::operator()(const Params params_input) {
		params_ = params_input;
		params_.clean(O);
		op_count_ = 0;
		// count_total_has_nots_ = 0;
		{
			std::lock_guard lock_guard {shared_mt_rng_mutex_};
			rng_.seed(shared_mt_rng_());
			for (auto& row : cells_) {
				std::shuffle(row.begin(), row.end(), shared_mt_rng_);
			}
			// TODO should the shuffle just use `rng_`? The data-parallel implementation would be much better that way.
		}
		// for (auto& counts : blks_has_) { counts.fill(0); }
		// for (auto& counts : cols_has_) { counts.fill(0); }
		// for (ord2i_t row {0}; row < O2; ++row) {
		// 	for (ord2i_t col {0}; col < O2; ++col) {
		// 		const ord2i_t val {cells_[row][col]};
		// 		++(blks_has_[rmi_to_blk<O>(row, col)][val]);
		// 		++(cols_has_[col][val]);
		// }	}
		// for (const auto& house : blks_has_) {
		// 	for (const auto& count : house) {
		// 		if (count == 0) { ++count_total_has_nots_; }
		// }	}
		// for (const auto& house : cols_has_) {
		// 	for (const auto& count : house) {
		// 		if (count == 0) { ++count_total_has_nots_; }
		// }	}
		is_done_ = false;
		this->generate_();
	}


	template<Order O>
	void GeneratorO<O>::continue_prev() {
		if (this->status() == ExitStatus::Ok) /* [[unlikely]] */ {
			return;
		}
		op_count_ = 0;
		this->generate_();
	}


	template<Order O>
	GeneratorO<O>::ord2i_t GeneratorO<O>::get_val_at_(const GeneratorO<O>::ord4x_t coord) const noexcept {
		return cells_[coord/O2][coord%O2];
	}


	template<Order O>
	void GeneratorO<O>::generate_() {
		using chute_has_counts_t = std::array<std::array<ord2x_t, O2>, O1>;

		for (ord2i_t h_chute {0}; h_chute < O2; h_chute += O1) {
			chute_has_counts_t blks_has_counts {{0}};
			for (ord2i_t row {h_chute}; row < h_chute+O1; ++row) {
				for (ord2i_t col {0}; col < O2; ++col) {
					++(blks_has_counts[col/O1][cells_[row][col]]);
			}	}
			int has_nots {0};
			for (const auto& blk_has_counts : blks_has_counts) {
				for (const auto& val_count : blk_has_counts) {
					if (val_count == 0) { ++has_nots; }
			}	}
			while (has_nots != 0) [[likely]] {
				const ord2x_t a_col {static_cast<ord2x_t>(static_cast<ord2x_t>(rng_() - rng_.min()) % O2)};
				const ord2x_t b_col {static_cast<ord2x_t>(static_cast<ord2x_t>(rng_() - rng_.min()) % O2)};
				const ord2x_t a_blk {static_cast<ord2x_t>(a_col/O1)};
				const ord2x_t b_blk {static_cast<ord2x_t>(b_col/O1)};
				if (a_blk == b_blk) [[unlikely]] { continue; }
				const ord2x_t row {static_cast<ord2x_t>(h_chute + (static_cast<ord2x_t>(rng_() - rng_.min()) % (O1/* -1 for some reason excluding this helps */)))};
				auto& a_cell = cells_[row][a_col];
				auto& b_cell = cells_[row][b_col];
				const int has_nots_diff {
					(blks_has_counts[a_blk][a_cell] == 1 ?  1 : 0) +
					(blks_has_counts[a_blk][b_cell] == 0 ? -1 : 0) +
					(blks_has_counts[b_blk][b_cell] == 1 ?  1 : 0) +
					(blks_has_counts[b_blk][a_cell] == 0 ? -1 : 0)
				};
				if (has_nots_diff < 0) [[unlikely]] /* TODO manually profile likelihood */ {
					has_nots += has_nots_diff;
					--blks_has_counts[a_blk][a_cell];
					++blks_has_counts[a_blk][b_cell];
					--blks_has_counts[b_blk][b_cell];
					++blks_has_counts[b_blk][a_cell];
					std::swap(a_cell, b_cell);
				}
				++op_count_;
				if (op_count_ >= params_.max_ops) {
					return;
				}
			}
		}

		for (ord2i_t v_chute {0}; v_chute < O2; v_chute += O1) {
			chute_has_counts_t cols_has_counts {{0}};
			for (ord2i_t row {0}; row < O2; ++row) {
				for (ord2i_t blk_col {0}; blk_col < O1; ++blk_col) {
					++(cols_has_counts[blk_col][cells_[row][v_chute+blk_col]]);
			}	}
			int has_nots {0};
			for (const auto& col_has_counts : cols_has_counts) {
				for (const auto& val_count : col_has_counts) {
					if (val_count == 0) { ++has_nots; }
			}	}
			while (has_nots != 0) [[likely]] {
				const ord2x_t a_col {static_cast<ord2x_t>(v_chute + (static_cast<ord2x_t>(rng_() - rng_.min()) % O1))};
				const ord2x_t b_col {static_cast<ord2x_t>(v_chute + (static_cast<ord2x_t>(rng_() - rng_.min()) % O1))};
				if (a_col == b_col) [[unlikely]] { continue; }
				const ord2x_t row {static_cast<ord2x_t>(static_cast<ord2x_t>(rng_() - rng_.min()) % (O1*(O1/* -1 */)))};
				auto& a_cell = cells_[row][a_col];
				auto& b_cell = cells_[row][b_col];
				const int has_nots_diff {
					(cols_has_counts[a_col][a_cell] == 1 ?  1 : 0) +
					(cols_has_counts[a_col][b_cell] == 0 ? -1 : 0) +
					(cols_has_counts[b_col][b_cell] == 1 ?  1 : 0) +
					(cols_has_counts[b_col][a_cell] == 0 ? -1 : 0)
				};
				if (has_nots_diff < 0) [[unlikely]] /* TODO manually profile likelihood */ {
					has_nots += has_nots_diff;
					--cols_has_counts[a_col][a_cell];
					++cols_has_counts[a_col][b_cell];
					--cols_has_counts[b_col][b_cell];
					++cols_has_counts[b_col][a_cell];
					std::swap(a_cell, b_cell);
				}
				++op_count_;
				if (op_count_ >= params_.max_ops) {
					return;
				}
			}
			{

			chute_has_counts_t cols_has_counts1 {{0}};
			for (ord2i_t row {0}; row < O2; ++row) {
				for (ord2i_t blk_col {0}; blk_col < O1; ++blk_col) {
					++(cols_has_counts1[blk_col][cells_[row][v_chute+blk_col]]);
			}	}
			int has_nots1 {0};
			for (const auto& col_has_counts : cols_has_counts1) {
				for (const auto& val_count : col_has_counts) {
					if (val_count == 0) { ++has_nots1; }
			}	}
			assert(has_nots1 == 0);
			}
		}
			std::cout << "hi\n\n\n\n" << std::endl;
		is_done_ = true;
		#ifndef NDEBUG
		std::array<ord2i_t, O4> grid;
		this->write_to_(std::span(grid));
		assert(is_sudoku_valid<O>(grid));
		#endif
	}


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template class GeneratorO<O_>;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}