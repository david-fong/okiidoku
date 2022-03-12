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
		// std::array<std::array<ord2x_t, O2>, O1> has_counts {0};
		// {
		// 	for (auto& counts : has_counts) { counts.fill(0); }
		// 	for (ord2i_t h_chute {0}; h_chute < O1; ++h_chute) {
		// 		for (ord2i_t row {0}; row < O2; ++row) {
		// 			for (ord2i_t col {0}; col < O2; ++col) {
		// 				const ord2i_t val {cells_[row][col]};
		// 				++(blks_has_[rmi_to_blk<O>(row, col)][val]);
		// 		}	}
		// 	}
		// }
		for (auto& counts : blks_has_) { counts.fill(0); }
		for (ord2i_t row {0}; row < O2; ++row) {
			for (ord2i_t col {0}; col < O2; ++col) {
				const ord2i_t val {cells_[row][col]};
				++(blks_has_[rmi_to_blk<O>(row, col)][val]);
		}	}
		for (const auto& house : blks_has_) {
			for (const auto& count : house) {
				if (count == 0) { ++count_total_has_nots_; }
		}	}
		while (count_total_has_nots_ != 0) [[likely]] {
			const ord2x_t row   {static_cast<ord2x_t>((rng_() - rng_.min()) % O2)};
			const ord2x_t a_col {static_cast<ord2x_t>((rng_() - rng_.min()) % O2)};
			const ord2x_t b_col {static_cast<ord2x_t>(( a_col + 1 + ((rng_() - rng_.min()) % (O2-1)) ) % O2)};
			const ord2x_t a_blk {rmi_to_blk<O>(row, a_col)};
			const ord2x_t b_blk {rmi_to_blk<O>(row, b_col)};
			auto& a_cell = cells_[row][a_col];
			auto& b_cell = cells_[row][b_col];
			assert ((a_col != b_col) && (a_cell != b_cell));
			int has_nots_diff {0
				// (cols_has_[a_col][a_cell] == 1 ?  1 : 0) +
				// (cols_has_[a_col][b_cell] == 0 ? -1 : 0) +
				// (cols_has_[b_col][b_cell] == 1 ?  1 : 0) +
				// (cols_has_[b_col][a_cell] == 0 ? -1 : 0)
			};
			if (a_blk != b_blk) [[likely]] {
				has_nots_diff +=
				(blks_has_[a_blk][a_cell] == 1 ?  1 : 0) +
				(blks_has_[a_blk][b_cell] == 0 ? -1 : 0) +
				(blks_has_[b_blk][b_cell] == 1 ?  1 : 0) +
				(blks_has_[b_blk][a_cell] == 0 ? -1 : 0);
			}
			if (has_nots_diff < 0) [[unlikely]] /* TODO manually profile likelihood */ {
				count_total_has_nots_ += has_nots_diff;
				// --cols_has_[a_col][a_cell];
				// ++cols_has_[a_col][b_cell];
				// --cols_has_[b_col][b_cell];
				// ++cols_has_[b_col][a_cell];
				if (a_blk != b_blk) [[likely]] {
					--blks_has_[a_blk][a_cell];
					++blks_has_[a_blk][b_cell];
					--blks_has_[b_blk][b_cell];
					++blks_has_[b_blk][a_cell];
				}
				std::swap(a_cell, b_cell);
			}
			++op_count_;
			if (op_count_ >= params_.max_ops) {
				return;
			}
		}
		std::cout << "hi" << std::endl;
		for (auto& counts : cols_has_) { counts.fill(0); }
		for (ord2i_t row {0}; row < O2; ++row) {
			for (ord2i_t col {0}; col < O2; ++col) {
				const ord2i_t val {cells_[row][col]};
				++(cols_has_[col][val]);
		}	}
		for (const auto& house : cols_has_) {
			for (const auto& count : house) {
				if (count == 0) { ++count_total_has_nots_; }
		}	}
		while (count_total_has_nots_ != 0) [[likely]] {
			const ord2x_t row   {static_cast<ord2x_t>((rng_() - rng_.min()) % O2)};
			const ord2x_t blk   {static_cast<ord2x_t>((O1 * (rng_() - rng_.min()) % O1))};
			const ord2x_t a_blk_col {static_cast<ord2x_t>((rng_() - rng_.min()) % O1)};
			const ord2x_t a_col {static_cast<ord2x_t>(blk + a_blk_col)};
			const ord2x_t b_col {static_cast<ord2x_t>(blk + (( a_blk_col + 1 + ((rng_() - rng_.min()) % (O1-1)) ) % O1))};
			// const ord2x_t a_blk {rmi_to_blk<O>(row, a_col)};
			// const ord2x_t b_blk {rmi_to_blk<O>(row, b_col)};
			auto& a_cell = cells_[row][a_col];
			auto& b_cell = cells_[row][b_col];
			assert ((a_col != b_col) && (a_cell != b_cell));
			int has_nots_diff {
				(cols_has_[a_col][a_cell] == 1 ?  1 : 0) +
				(cols_has_[a_col][b_cell] == 0 ? -1 : 0) +
				(cols_has_[b_col][b_cell] == 1 ?  1 : 0) +
				(cols_has_[b_col][a_cell] == 0 ? -1 : 0)
			};
			if (has_nots_diff < 0) [[unlikely]] /* TODO manually profile likelihood */ {
				count_total_has_nots_ += has_nots_diff;
				--cols_has_[a_col][a_cell];
				++cols_has_[a_col][b_cell];
				--cols_has_[b_col][b_cell];
				++cols_has_[b_col][a_cell];
				std::swap(a_cell, b_cell);
			}
			++op_count_;
			if (op_count_ >= params_.max_ops) {
				return;
			}
		}
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