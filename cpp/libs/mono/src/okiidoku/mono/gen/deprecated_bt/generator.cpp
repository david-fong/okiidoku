#include <okiidoku/mono/gen/deprecated_bt/generator.hpp>
#include <okiidoku/shared_rng.hpp>

#include <algorithm> // shuffle

namespace okiidoku::mono::gen::bt {

	// long long total = 0;
	// long long true_ = 0;

	Params Params::clean(const Order O) noexcept {
		if (max_dead_ends == 0) {
			max_dead_ends = cell_dead_ends::limit_default[O];
		} else if (max_dead_ends > cell_dead_ends::limit_i_max[O]) {
			max_dead_ends = cell_dead_ends::limit_i_max[O];
		}
		return *this;
	}


	std::unique_ptr<Generator> Generator::create(const Order order) {
		switch (order) {
			#define OKIIDOKU_FOR_COMPILED_O(O_) \
			case O_: return std::make_unique<gen::bt::Generator<O_>>();
			OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
			#undef OKIIDOKU_FOR_COMPILED_O

			default: return std::make_unique<gen::bt::Generator<OKIIDOKU_DEFAULT_ORDER>>();
		}
	}


	template<Order O>
	void Generator<O>::operator()(const Params params_input) {
		params_ = params_input;
		params_.clean(O);
		
		for (Cell& cell : cells_) {
			cell.clear();
		}
		rows_has_.fill(0);
		cols_has_.fill(0);
		boxes_has_.fill(0);
		dead_ends_.fill(0);

		progress_ = 0;
		backtrack_origin_ = 0;
		most_dead_ends_seen_ = 0;
		op_count_ = 0;
		{
			std::lock_guard lock_guard {shared_rng.mutex};
			for (auto& vto : val_try_orders_) {
				std::shuffle(vto.begin(), vto.end(), shared_rng.rng);
			}
		}

		this->generate_();
	}


	template<Order O>
	void Generator<O>::continue_prev() {
		if (this->status() == ExitStatus::exhausted) [[unlikely]] {
			return;
		}
		dead_ends_.fill(0); // Do not carry over previous dead_ends counters.
		most_dead_ends_seen_ = 0;
		this->generate_();
	}


	template<Order O>
	Generator<O>::o2i_t Generator<O>::extract_val_at_(const Generator<O>::o4x_t coord) const noexcept {
		const auto p = path::get_coord_to_prog_converter<O>(params_.path_kind)(coord);
		const auto try_index = cells_[p].try_index;
		if (try_index == O2) { return O2; }
		return val_try_orders_[p/O2][try_index];
	}


	template<Order O>
	Generator<O>::dead_ends_t Generator<O>::extract_dead_ends_at_(const o4x_t coord) const noexcept {
		return dead_ends_[path::get_coord_to_prog_converter<O>(params_.path_kind)(coord)];
	}


	template<Order O>
	void Generator<O>::generate_() {
		// see the inline-brute-force-func git branch for experimenting with manually inlining set_next_valid_
		const path::coord_converter<O> prog_to_coord = path::get_prog_to_coord_converter<O>(params_.path_kind);

		bool backtracked = op_count_ != 0;
		while (true) [[likely]] {
			const Direction direction = this->set_next_valid_(prog_to_coord, backtracked);
			backtracked = direction.is_back;
			if (!direction.is_back_skip) [[likely]] { ++op_count_; }

			if (direction.is_back) [[unlikely]] {
				if (progress_ == 0) [[unlikely]] {
					progress_ = O4;
					break;
				}
				if (!direction.is_back_skip) [[unlikely]] {
					const dead_ends_t dead_ends = ++dead_ends_[progress_];
					if (dead_ends > most_dead_ends_seen_) [[unlikely]] {
						most_dead_ends_seen_ = dead_ends;
						if (dead_ends >= params_.max_dead_ends) [[unlikely]] {
							--progress_;
							break;
						}
					}
				}
				--progress_;
			} else {
				if (progress_ == O4-1) [[unlikely]] {
					break;
				}
				++progress_;
			}
		}
		#ifndef NDEBUG
		grid_arr_t<O> grid;
		this->write_to_(std::span(grid));
		assert(grid_follows_rule<O>(grid));
		#endif
	}


	template<Order O>
	Generator<O>::Direction Generator<O>::set_next_valid_(path::coord_converter<O> prog_to_coord, const bool backtracked) noexcept {
		const o4x_t coord = prog_to_coord(progress_);
		has_mask_t& row_has = rows_has_[rmi_to_row<O>(coord)];
		has_mask_t& col_has = cols_has_[rmi_to_col<O>(coord)];
		has_mask_t& box_has = boxes_has_[rmi_to_box<O>(coord)];
		const auto& val_try_order = val_try_orders_[progress_ / O2];

		Cell& cell = cells_[progress_];
		if (backtracked) [[unlikely]]/* average direction is forward */ {
			// Clear the current value from all masks:
			// Note: the more readable way of writing this is less performant :/
			const has_mask_t erase_mask {~(has_mask_t{1} << val_try_order[cell.try_index])};
			row_has &= erase_mask;
			col_has &= erase_mask;
			box_has &= erase_mask;

			// Smart skip-backtracking:
			// This optimization's degree of usefulness depends on the genpath and size.
			if (!cells_share_house<O>(coord, prog_to_coord(backtrack_origin_))) [[unlikely]] {
				// only likely when deal_row_major. deal_row_major is so slow already
				// it doesn't feel worth slowing other genpaths down to microoptimize.
				cell.clear();
				return Direction { .is_back = true, .is_back_skip = true };
			}
		}

		const has_mask_t cell_has {row_has | col_has | box_has};
		if (!cell_has.all()) [[likely]] {
			// The above optimization comes into effect ~1/5 of the time for size 5.
			for (o2i_t try_i {static_cast<o2i_t>((cell.try_index+1u) % (O2+1))}; try_i < O2; ++try_i) [[likely]] {
				// Note: the more readable way of writing this is less performant :/
				const has_mask_t try_val_mask = has_mask_t{1} << val_try_order[try_i];
				if ((cell_has & try_val_mask).none()) [[unlikely]] {
					// A valid value was found:
					row_has |= try_val_mask;
					col_has |= try_val_mask;
					box_has |= try_val_mask;
					cell.try_index = try_i;
					return Direction { .is_back = false, .is_back_skip = false };
				}
			}
		}
		// Nothing left to try here. Backtrack:
		cell.clear();
		backtrack_origin_ = progress_;
		return Direction { .is_back = true, .is_back_skip = false };
	}


	#define OKIIDOKU_FOR_COMPILED_O(O_) \
		template class Generator<O_>;
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}