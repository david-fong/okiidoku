#include "solvent/gen/stochastic.hpp"
#include "solvent/print.hpp"
#include "solvent/rng.hpp"

#include <algorithm> // swap, shuffle

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
		count_total_has_nots_ = 0;
		{
			std::lock_guard lock_guard {shared_mt_rng_mutex_};
			rng_.seed(shared_mt_rng_());
			for (auto& row : cells_) {
				std::shuffle(row.begin(), row.end(), shared_mt_rng_);
			}
			// TODO should the shuffle just use `rng_`? The data-parallel implementation would be much better that way.
		}
		for (auto counts : blks_has_) { counts.fill(0); }
		for (auto counts : cols_has_) { counts.fill(0); }
		for (ord2i_t row {0}; row < O2; ++row) {
			for (ord2i_t col {0}; col < O2; ++col) {
				const ord2i_t val {cells_[row][col]};
				++(blks_has_[rmi_to_blk<O>(row, col)][val]);
				++(cols_has_[col][val]);
		}	}
		for (const auto& house : blks_has_) {
			for (const auto& count : house) {
				if (count == 0) { ++count_total_has_nots_; }
		}	}
		for (const auto& house : cols_has_) {
			for (const auto& count : house) {
				if (count == 0) { ++count_total_has_nots_; }
		}	}
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
		while (count_total_has_nots_ != 0) [[likely]] {
			const ord2x_t row   {static_cast<ord2x_t>(rng_() % O2)};
			const ord2x_t col_a {static_cast<ord2x_t>(rng_() % O2)};
			const ord2x_t col_b {static_cast<ord2x_t>(col_a + (rng_() % (O2-1)) % O2)};
			auto& cell_a = cells_[row][col_a];
			auto& cell_b = cells_[row][col_b];
			// assert (col_a != col_b) && (cell_a != cell_b)
			const int has_nots_diff = (
				(blks_has_[rmi_to_blk<O>(row, col_a)][cell_a] == 1 ?  1 : 0) +
				(blks_has_[rmi_to_blk<O>(row, col_a)][cell_b] == 0 ? -1 : 0) +
				(blks_has_[rmi_to_blk<O>(row, col_b)][cell_b] == 1 ?  1 : 0) +
				(blks_has_[rmi_to_blk<O>(row, col_b)][cell_a] == 0 ? -1 : 0) +
				(cols_has_[col_a][cell_a] == 1 ?  1 : 0) +
				(cols_has_[col_a][cell_b] == 0 ? -1 : 0) +
				(cols_has_[col_b][cell_b] == 1 ?  1 : 0) +
				(cols_has_[col_b][cell_a] == 0 ? -1 : 0)
			);
			if (has_nots_diff < 0) [[unlikely]] /* TODO manually profile likelihood */ {
				count_total_has_nots_ += has_nots_diff;
				std::swap(cell_a, cell_b);
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