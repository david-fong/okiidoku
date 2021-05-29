#include "mod.hpp"

#include <random>
#include <mutex>
#include <algorithm>   // random_shuffle,
#include <numeric>     // iota,


namespace solvent::lib::gen {

	// Guards accesses to Rng. I currently only
	// use this when shuffling generator biases.
	std::mutex RNG_MUTEX;


	template<Order O>
	Generator<O>::Generator()
	{
		for (auto& vto : val_try_order_) {
			std::iota(vto.begin(), vto.end(), 0);
		}
	}


	template<Order O>
	void Generator<O>::init(void) {
		for (auto& t : values_) {
			t.clear();
		}
		rows_has_.fill(0);
		cols_has_.fill(0);
		blks_has_.fill(0);
		backtracks_.fill(0);

		// Scramble each row's value-guessing-O1:
		RNG_MUTEX.lock();
		for (auto& vto : val_try_order_) {
			std::shuffle(vto.begin(), vto.end(), Rng);
		}
		RNG_MUTEX.unlock();
	}


	template<Order O>
	Generator<O>::GenResult Generator<O>::generate(const std::optional<Params> params) {
		GenResult info;
		if (params) [[likely]] {
			info.params = params.value();
			this->init();
		} else {
			info = gen_result_;
			if (info.status == ExitStatus::Exhausted) [[unlikely]] {
				return info;
			}
		}
		ord4_t (& prog2coord)(ord4_t) = path::PathCoords<O>[info.params.path_kind];
		ord4_t dead_end_progress = info.progress;

		while (info.progress < O4) {;
			const bool do_backtrack = set_next_valid(info.progress);
			++info.op_count;
			if (do_backtrack) [[unlikely]] {
				if (info.progress == 0) [[unlikely]] {
					info.status = ExitStatus::Exhausted;
					break;
				}
				const backtrack_t backtracks = ++backtracks_[info.progress];
				--info.progress;
				if (backtracks > info.most_backtracks_seen) [[unlikely]] {
					info.most_backtracks_seen = backtracks;
					if (info.most_backtracks_seen >= info.params.most_backtracks) [[unlikely]] {
						info.status = ExitStatus::Abort;
						break;
					}
				}
			} else {
				++info.progress;
				if (!can_coords_see_each_other(prog2coord(info.progress), prog2coord(dead_end_progress))
					|| (info.progress > dead_end_progress)
				) {
					++dead_end_progress;
				}
			}
		}
		if (info.progress == O4) [[likely]] {
			// [[likely]] helps for small grids and barely affects large ones.
			info.status = ExitStatus::Ok;
			--info.progress;
		}
		for (ord4_t i = 0; i < O4; i++) {
			info.grid[prog2coord(i)] = values_[i].value;
		}
		gen_result_ = info;
		return info;
	}


	template<Order O>
	bool Generator<O>::set_next_valid(const ord4_t progress) noexcept {
		const ord4_t coord = prog2coord(info.progress);
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

		const has_mask_t t_has = (row_has | col_has | blk_has);
		// NOTE: these do not improve time-scaling performance, but I wish they did.
		/*
		if (std::popcount(t_has) == O2) [[unlikely]] {
			t.clear();
			return PathDirection::Back;
		} else if (std::popcount(t_has) == O2 - 1) {
			const ord2_t try_val = std::countl_zero(!t_has);
			const has_mask_t try_val_mask = 0b1 << try_val;
			row_has |= try_val_mask;
			col_has |= try_val_mask;
			blk_has |= try_val_mask;
			t.value = try_val;
			t.next_try_index = O2;
			return false;
		}
		*/
		for (ord2_t try_i = t.next_try_index; try_i < O2; try_i++) {
			const ord2_t try_val = val_try_order_[this->get_row(coord)][try_i];
			const has_mask_t try_val_mask = has_mask_t(1) << try_val;
			if (!(t_has & try_val_mask)) {
				// A valid value was found:
				row_has |= try_val_mask;
				col_has |= try_val_mask;
				blk_has |= try_val_mask;
				t.value = try_val;
				t.next_try_index = (try_i + 1u);
				return false;
			}
		}
		// Backtrack:
		t.clear();
		return true;
	}
}