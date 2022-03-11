#include "solvent/gen/stochastic.hpp"
#include "solvent/print.hpp"
#include "solvent/rng.hpp"

#include <algorithm> // shuffle,

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
			rng_.seed(shared_mt_rng_)
			for (auto& row : cells_) {
				std::shuffle(row.begin(), row.end(), shared_mt_rng_);
			}
			// TODO should the shuffle just use `rng_`? The data-parallel implementation would be much better that way.
		}
		blks_has_.fill(0);
		cols_has_.fill(0);
		for (ord2i_t row {0}; row < O2; ++row) {
			for (ord2i_t col {0}; col < O2; ++col) {
				ord2i_t = cells_[row][col];
				++blks_has_[rmi_to_blk(row, col)][val];
				++cols_has_[col][val]
		}	}
		this->generate_();
	}


	template<Order O>
	void GeneratorO<O>::continue_prev() {
		if (this->status() == ExitStatus::Ok) /* [[unlikely]] */ {
			this->operator()(params_);
			return;
		}
		op_count_ = 0;
		this->generate_();
	}


	template<Order O>
	GeneratorO<O>::ord2i_t GeneratorO<O>::extract_val_at_(const GeneratorO<O>::ord4x_t coord) const noexcept {
		return cells_[coord/O2][coord%O2];
	}


	template<Order O>
	void GeneratorO<O>::generate_() {
		while (count_total_has_nots_ != 0) [[likely]] {
			// TODO
		}
		#ifndef NDEBUG
		std::array<ord2i_t, O4> grid;
		this->write_to_(std::span(grid));
		assert(is_sudoku_valid<O>(grid));
		#endif
	}


	template<Order O>
	void GeneratorO<O>::try_swap_() noexcept {
		// TODO
	}


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template class GeneratorO<O_>;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}