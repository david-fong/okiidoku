#include <solvent_lib/gen/mod.hpp>
#include <solvent_lib/print.hpp>
#include <solvent_util/str.hpp>

#include <iostream>
#include <mutex>
#include <algorithm>   // shuffle,
#include <numeric>     // iota,


namespace solvent::lib::gen {

	std::mt19937 Rng;

	// Guards accesses to Rng. I currently only
	// use this when shuffling generator biases.
	std::mutex RNG_MUTEX;


	template<Order O>
	Params Params::clean(void) noexcept {
		if (max_dead_ends == 0) {
			max_dead_ends = Generator<O>::DEFAULT_MAX_DEAD_ENDS;
		}
		return *this;
	}


	template<Order O>
	Generator<O>::Generator(void)
	{
		for (auto& vto : val_try_orders_) {
			std::iota(vto.begin(), vto.end(), 0);
		}
	}


	template<Order O>
	void Generator<O>::init(void) {
		for (Tile& t : values_) {
			t.clear();
		}
		rows_has_.fill(0);
		cols_has_.fill(0);
		blks_has_.fill(0);
		dead_ends_.fill(0);

		RNG_MUTEX.lock();
		for (auto& vto : val_try_orders_) {
			std::shuffle(vto.begin(), vto.end(), Rng);
		}
		RNG_MUTEX.unlock();
	}


	template<Order O>
	Generator<O>::GenResult Generator<O>::generate(const std::optional<Params> params) {
		GenResult info;
		if (params.has_value()) [[likely]] {
			info.params = params.value();
			info.params.template clean<O>();
			this->init();
		} else {
			// Continue the previous generation.
			info = gen_result_;
			if (info.status == ExitStatus::Exhausted) [[unlikely]] {
				return info;
			}
			dead_ends_.fill(0);
		}
		ord4_t (& prog2coord)(ord4_t) = path::GetPathCoords<O>(info.params.path_kind);
		ord4_t& dead_end_progress = info.dead_end_progress;

		while (true) {
			const Direction direction = this->set_next_valid(
				info.progress, dead_end_progress, prog2coord
			);
			if (!direction.is_skip) { ++info.op_count; }

			if (direction.is_back) [[unlikely]] {
				if (info.progress == 0) [[unlikely]] {
					info.status = ExitStatus::Exhausted;
					break;
				}
				if (info.progress == dead_end_progress) [[unlikely]] {
					const dead_ends_t dead_ends = ++dead_ends_[info.progress];
					--info.progress;
					if (dead_ends > info.most_dead_ends_seen) [[unlikely]] {
						info.most_dead_ends_seen = dead_ends;
						if (info.most_dead_ends_seen > info.params.max_dead_ends) [[unlikely]] {
							info.status = ExitStatus::Abort;
							break;
						}
					}
				} else {
					--info.progress;
				}
			} else {
				if (info.progress == O4-1) [[unlikely]] {
					info.status = ExitStatus::Ok;
					break;
				}
				++info.progress;
				if ((info.progress > dead_end_progress)
					|| !this->can_coords_see_each_other(prog2coord(info.progress), prog2coord(dead_end_progress))
				) [[unlikely]] { // TODO.learn `unlikely` helps for 4:rowmajor. Does it help in general?
					dead_end_progress = info.progress;
				}
			}
		}

		for (ord4_t i = 0; i < O4; i++) {
			info.grid[prog2coord(i)] = values_[i].value;
		}
		gen_result_ = info;
		return info;
	}


	template<Order O>
	Direction Generator<O>::set_next_valid(
		const ord4_t progress, const ord4_t dead_end_progress, ord4_t (& prog2coord)(ord4_t)
	) noexcept {
		const ord4_t coord = prog2coord(progress);
		has_mask_t& row_has = rows_has_[this->get_row(coord)];
		has_mask_t& col_has = cols_has_[this->get_col(coord)];
		has_mask_t& blk_has = blks_has_[this->get_blk(coord)];

		Tile& t = values_[progress];
		if (!t.is_clear()) {
			// Clear the current value from all masks:
			const has_mask_t erase_mask = ~(has_mask_t(0b1u) << t.value);
			row_has &= erase_mask;
			col_has &= erase_mask;
			blk_has &= erase_mask;
		}

		// Smart backtracking:
		if ((progress < dead_end_progress) && !this->can_coords_see_each_other(
			prog2coord(progress), prog2coord(dead_end_progress)
		)) {
			t.clear();
			return Direction { .is_back = true, .is_skip = true };
		}

		const has_mask_t t_has = (row_has | col_has | blk_has);
		for (ord2_t try_i = t.next_try_index; try_i < O2; try_i++) {
			const ord2_t try_val = val_try_orders_[progress / O2][try_i];
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
		t.clear();
		return Direction { .is_back = true, .is_skip = false };
	}


	std::string shaded_dead_end_stat(const long out_of, const long count) {
		const unsigned int relative_intensity
			= static_cast<double>(count - 1)
			* util::str::BLOCK_CHARS.size()
			/ out_of;
		return (count == 0) ? " " : util::str::BLOCK_CHARS[relative_intensity];
	}


	template<Order O>
	void Generator<O>::GenResult::print_serial(std::ostream& os) const {
		const print::grid_t grid_accessor([this](uint16_t coord) { return this->grid[coord]; });
		print::serial(os, O, grid_accessor);
	}


	template<Order O>
	void Generator<O>::GenResult::print_pretty(std::ostream& os) const {
		const std::vector<print::grid_t> grid_accessors = {
			print::grid_t([this](uint16_t coord) { return this->grid[coord]; })
		};
		print::pretty(os, O, grid_accessors);
	}


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template Params Params::clean<O_>(void) noexcept; \
		template class Generator<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL
}