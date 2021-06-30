#include <solvent_lib/gen/mod.hpp>
#include <solvent_lib/print.hpp>
#include <solvent_util/str.hpp>

#include <iostream>
#include <mutex>
#include <algorithm>   // shuffle,


namespace solvent::lib::gen {

	std::mt19937 Rng;

	// Guards accesses to Rng. I currently only
	// use this when shuffling generator biases.
	std::mutex RNG_MUTEX;


	Params Params::clean(const Order O) noexcept {
		if (max_dead_ends == 0) {
			max_dead_ends = DEFAULT_MAX_DEAD_ENDS(O);
		}
		return *this;
	}


	template<Order O>
	typename size<O>::ord2_t Generator<O>::operator[](const ord4_t coord) const {
		typename path::coord_converter_t<O> prog2coord = path::GetPathCoords<O>(params_.path_kind);
		return values_[prog2coord(coord)].value;
	}


	template<Order O>
	GenResult Generator<O>::operator()(const Params params) {
		params_ = params;
		params_.clean(O);
		this->prepare_fresh_gen();
		this->generate();
		return this->make_gen_result();
	}


	template<Order O>
	GenResult Generator<O>::continue_prev() {
		// Continue the previous generation.
		if (prev_gen_status_ == ExitStatus::Exhausted) [[unlikely]] {
			return this->make_gen_result();
		}
		dead_ends_.fill(0); // Do not carry over previous dead_ends counters.
		most_dead_ends_seen_ = 0;
		this->generate();
		return this->make_gen_result();
	}


	template<Order O>
	void Generator<O>::prepare_fresh_gen(void) {
		for (Tile& t : values_) {
			t.clear();
		}
		rows_has_.fill(0);
		cols_has_.fill(0);
		blks_has_.fill(0);
		dead_ends_.fill(0);

		progress_ = 0;
		dead_end_progress_ = 0;
		most_dead_ends_seen_ = 0;
		op_count_ = 0;

		RNG_MUTEX.lock();
		for (auto& vto : val_try_orders_) {
			std::shuffle(vto.begin(), vto.end(), Rng);
		}
		RNG_MUTEX.unlock();
	}


	template<Order O>
	void Generator<O>::generate(void) {
		typename path::coord_converter_t<O> prog2coord = path::GetPathCoords<O>(params_.path_kind);

		while (true) {
			const Direction direction = this->set_next_valid(prog2coord);
			if (!direction.is_skip) { ++op_count_; }

			if (direction.is_back) [[unlikely]] {
				if (progress_ == 0) [[unlikely]] {
					prev_gen_status_ = ExitStatus::Exhausted;
					break;
				}
				if (progress_ == dead_end_progress_) [[unlikely]] {
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
				if ((progress_ > dead_end_progress_)
					|| !this->can_coords_see_each_other(prog2coord(progress_), prog2coord(dead_end_progress_))
				) [[unlikely]] { // TODO.learn `unlikely` helps for 4:rowmajor. Does it help in general?
					dead_end_progress_ = progress_;
				}
			}
		}
	}


	template<Order O>
	Direction Generator<O>::set_next_valid(typename path::coord_converter_t<O> prog2coord) noexcept {
		const ord4_t coord = prog2coord(progress_);
		has_mask_t& row_has = rows_has_[this->get_row(coord)];
		has_mask_t& col_has = cols_has_[this->get_col(coord)];
		has_mask_t& blk_has = blks_has_[this->get_blk(coord)];

		Tile& t = values_[progress_];
		if (!t.is_clear()) {
			// Clear the current value from all masks:
			const has_mask_t erase_mask = ~(has_mask_t(0b1u) << t.value);
			row_has &= erase_mask;
			col_has &= erase_mask;
			blk_has &= erase_mask;
		}

		// Smart backtracking:
		if ((progress_ < dead_end_progress_) && !this->can_coords_see_each_other(
			prog2coord(progress_), prog2coord(dead_end_progress_)
		)) {
			t.clear();
			return Direction { .is_back = true, .is_skip = true };
		}

		const has_mask_t t_has = (row_has | col_has | blk_has);
		for (ord2_t try_i = t.next_try_index; try_i < O2; try_i++) {
			const ord2_t try_val = val_try_orders_[progress_ / O2][try_i];
			const has_mask_t try_val_mask = has_mask_t(1) << try_val;
			if (!(t_has & try_val_mask)) {
				// A valid value was found:
				row_has |= try_val_mask;
				col_has |= try_val_mask;
				blk_has |= try_val_mask;
				t.value = try_val;
				t.next_try_index = (try_i + 1u);
				return Direction { .is_back = false };
			}
		}
		// Nothing left to try here. Backtrack:
		t.clear(); // TODO.test is this needed? Maybe not!
		return Direction { .is_back = true, .is_skip = false };
	}


	template<Order O>
	GenResult Generator<O>::make_gen_result(void) const {
		typename path::coord_converter_t<O> prog2coord = path::GetPathCoords<O>(params_.path_kind);
		GenResult gen_result = {
			.O {O},
			.status {prev_gen_status_},
			.dead_end_progress {dead_end_progress_},
			.most_dead_ends_seen {most_dead_ends_seen_},
			.op_count {op_count_}
		};
		gen_result.grid.resize(O4);
		for (ord4_t i = 0; i < O4; i++) {
			gen_result.grid[prog2coord(i)] = values_[i].value;
		}
		return gen_result;
	}


	std::string shaded_dead_end_stat(const long out_of, const long count) {
		const unsigned int relative_intensity
			= static_cast<double>(count - 1)
			* util::str::BLOCK_CHARS.size()
			/ out_of;
		return (count == 0) ? " " : util::str::BLOCK_CHARS[relative_intensity];
	}


	void GenResult::print_serial(std::ostream& os) const {
		const print::grid_t grid_accessor([this](uint16_t coord) { return this->grid[coord]; });
		print::serial(os, O, grid_accessor);
	}


	void GenResult::print_pretty(std::ostream& os) const {
		const std::vector<print::grid_t> grid_accessors = {
			print::grid_t([this](uint16_t coord) { return this->grid[coord]; })
		};
		print::pretty(os, O, grid_accessors);
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template class Generator<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}