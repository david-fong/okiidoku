#include <solvent_lib/gen/mod.hpp>
#include <solvent_lib/equiv/canon.hpp>
#include <solvent_lib/print.hpp>
#include <solvent_util/str.hpp>

#include <iostream>
#include <mutex>
#include <algorithm>   // shuffle,


namespace solvent::lib::gen {

	std::mt19937 Rng;

	// Guards accesses to Rng. Only used when shuffling generator biases.
	std::mutex RNG_MUTEX;


	Params Params::clean(const Order O) noexcept {
		if (max_dead_ends == 0) {
			max_dead_ends = DEFAULT_MAX_DEAD_ENDS(O);
		}
		return *this;
	}


	template<Order O>
	typename size<O>::ord2_t Generator<O>::operator[](const ord4_t coord) const {
		const auto try_index = cells_[coord].try_index;
		return (try_index == O2) ? O2 : val_try_orders_[coord / O2][try_index];
	}


	template<Order O>
	GenResult Generator<O>::operator()(const Params params) {
		params_ = params;
		params_.clean(O);
		this->prepare_fresh_gen_();
		this->generate_();
		return this->make_gen_result_();
	}


	template<Order O>
	GenResult Generator<O>::continue_prev(void) {
		// Continue the previous generation.
		if (prev_gen_status_ == ExitStatus::Exhausted) [[unlikely]] {
			return this->make_gen_result_();
		}
		dead_ends_.fill(0); // Do not carry over previous dead_ends counters.
		most_dead_ends_seen_ = 0;
		this->generate_();
		return this->make_gen_result_();
	}


	template<Order O>
	void Generator<O>::prepare_fresh_gen_(void) {
		for (Cell& cell : cells_) {
			cell.clear();
		}
		rows_has_.fill(0);
		cols_has_.fill(0);
		blks_has_.fill(0);
		dead_ends_.fill(0);

		progress_ = 0;
		frontier_progress_ = 0;
		most_dead_ends_seen_ = 0;
		op_count_ = 0;

		RNG_MUTEX.lock();
		for (auto& vto : val_try_orders_) {
			std::shuffle(vto.begin(), vto.end(), Rng);
		}
		RNG_MUTEX.unlock();
	}


	template<Order O>
	void Generator<O>::generate_(void) {
		typename path::coord_converter_t<O> prog2coord = path::GetPathCoords<O>(params_.path_kind);

		bool do_clear_masks = true;
		while (true) {
			const Direction direction = this->set_next_valid_(prog2coord, do_clear_masks);
			do_clear_masks = direction.is_back;
			if (!direction.is_skip) [[likely]] { ++op_count_; }

			if (direction.is_back) [[unlikely]] {
				if (progress_ == 0) [[unlikely]] {
					prev_gen_status_ = ExitStatus::Exhausted;
					break;
				}
				if (progress_ == frontier_progress_) [[unlikely]] {
					const dead_ends_t dead_ends = ++dead_ends_[progress_];
					--progress_;
					if (dead_ends > most_dead_ends_seen_) [[unlikely]] {
						most_dead_ends_seen_ = dead_ends;
						if (most_dead_ends_seen_ > params_.max_dead_ends) [[unlikely]] {
							prev_gen_status_ = ExitStatus::Abort;
							break;
						}
					}
				} else {
					--progress_;
				}
			} else {
				if (progress_ == O4-1) [[unlikely]] {
					prev_gen_status_ = ExitStatus::Ok;
					break;
				}
				++progress_;
				if ((progress_ > frontier_progress_)
					|| !this->cells_share_house(prog2coord(progress_), prog2coord(frontier_progress_))
				) [[unlikely]] {
					frontier_progress_ = progress_;
				}
			}
		}
	}


	template<Order O>
	Direction Generator<O>::set_next_valid_(typename path::coord_converter_t<O> prog2coord, const bool do_clear_masks) noexcept {
		const ord4_t coord = prog2coord(progress_);
		has_mask_t& row_has = rows_has_[this->get_row(coord)];
		has_mask_t& col_has = cols_has_[this->get_col(coord)];
		has_mask_t& blk_has = blks_has_[this->get_blk(coord)];
		auto& val_try_order = val_try_orders_[coord / O2];

		Cell& cell = cells_[coord];
		if (do_clear_masks) [[unlikely]]/* average direction is forward */ {
			// Clear the current value from all masks:
			const has_mask_t erase_mask = ~( has_mask_t(0b1u) << val_try_order[cell.try_index] );
			row_has &= erase_mask;
			col_has &= erase_mask;
			blk_has &= erase_mask;
		}

		// Smart backtracking:
		// This optimization's degree of usefulness depends on the genpath and size.
		if ((progress_ < frontier_progress_) && !this->cells_share_house(
			coord, prog2coord(frontier_progress_)
		)) [[unlikely]] {
			cell.clear();
			return Direction { .is_back = true, .is_skip = true };
		}

		const has_mask_t cell_has = (row_has | col_has | blk_has);
		if (std::popcount(cell_has) != O2) [[likely]] {
			// The above optimization comes into effect ~1/5 of the time for size 5.
			for (ord2_t try_i = (cell.try_index+1) % (O2+1); try_i < O2; try_i++) {
				const has_mask_t try_val_mask = has_mask_t(1) << val_try_order[try_i];
				if (!(cell_has & try_val_mask)) [[unlikely]] {
					// A valid value was found:
					row_has |= try_val_mask;
					col_has |= try_val_mask;
					blk_has |= try_val_mask;
					cell.try_index = try_i;
					return Direction { .is_back = false, .is_skip = false };
				}
			}
		}
		// Nothing left to try here. Backtrack:
		cell.clear();
		return Direction { .is_back = true, .is_skip = false };
	}


	template<Order O>
	GenResult Generator<O>::make_gen_result_(void) const {
		typename path::coord_converter_t<O> prog2coord = path::GetPathCoords<O>(params_.path_kind);
		GenResult gen_result = {
			.O {O},
			.status {prev_gen_status_},
			.frontier_progress {frontier_progress_},
			.most_dead_ends_seen {most_dead_ends_seen_},
			.op_count {op_count_},
			.grid = std::vector(O4, O2),
		};
		for (ord4_t p = 0; p <= progress_; p++) {
			const ord4_t coord = prog2coord(p);
			gen_result.grid[coord] = val_try_orders_[coord / O2][cells_[coord].try_index];
		}
		if (params_.canonicalize && gen_result.status == ExitStatus::Ok) /* [[unlikely]] (worth?) */ {
			gen_result.grid = equiv::canonicalize<O>(gen_result.grid);
		}
		return gen_result;
	}


	std::string shaded_dead_end_stat(const long out_of, const long count) {
		const unsigned int relative_intensity
			= static_cast<double>(count - 1)
			* util::str::BLOCK_CHARS.size()
			/ (out_of + 1);
		return (count == 0) ? " " : util::str::BLOCK_CHARS[relative_intensity];
	}


	void GenResult::print_serial(std::ostream& os) const {
		const print::val_grid_t grid_accessor([this](uint32_t coord) { return this->grid[coord]; });
		print::serial(os, O, grid_accessor);
	}


	void GenResult::print_pretty(std::ostream& os) const {
		const std::vector<print::print_grid_t> grid_accessors = {
			print::print_grid_t([this](std::ostream& os, uint16_t coord) {
				os << ' '; print::val2str(os, O, this->grid[coord]);
			}),
		};
		print::pretty(os, O, grid_accessors);
	}


	template<Order O>
	void Generator<O>::print_pretty(std::ostream& os) const {
		const std::vector<print::print_grid_t> grid_accessors = {
			print::print_grid_t([this](std::ostream& os, uint16_t coord) {
				os << ' '; print::val2str(os, O, operator[](coord));
			}),
			print::print_grid_t([this](std::ostream& os, uint16_t coord) {
				const auto shade = shaded_dead_end_stat(params_.max_dead_ends, dead_ends_[coord]);
				os << shade << shade;
			}),
		};
		print::pretty(os, O, grid_accessors);
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template class Generator<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}