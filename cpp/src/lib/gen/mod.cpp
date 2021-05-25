#include "./mod.hpp"

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
	Generator<O>::GenResult Generator<O>::generate(const std::optional<path::Kind> request) {
		GenResult info;
		if (request) [[likely]] {
			info.path_kind = request.value();
			this->init();
		} else {
			info = gen_result_;
			if (info.status == ExitStatus::Exhausted) [[unlikely]] {
				return info;
			}
		}
		ord4_t (& prog2coord)(ord4_t) = path::PathCoords<O>[info.path_kind];
		PathDirection direction;

		while (info.progress < O4) {
			const ord4_t coord = prog2coord(info.progress);
			direction = set_next_valid(info.progress, coord);
			info.op_count++;
			if (direction == PathDirection::Back) [[unlikely]] {
				// Pop and step backward:
				if (++backtracks_[info.progress] > info.most_backtracks) {
					info.most_backtracks = backtracks_[info.progress];
				}
				if (info.progress == 0) [[unlikely]] {
					info.status = ExitStatus::Exhausted;
					break;
				}
				--info.progress;
			} else {
				// (direction == PathDirection::Forward)
				++info.progress;
			}
			// Check whether the give-up-condition has been met:
			if (info.most_backtracks >= DEFAULT_MAX_BACKTRACKS) [[unlikely]] {
				info.status = ExitStatus::Abort;
				break;
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
	PathDirection Generator<O>::set_next_valid(const ord4_t progress, const ord4_t coord) {
		has_mask_t& row_has = rows_has_[this->get_row(coord)];
		has_mask_t& col_has = cols_has_[this->get_col(coord)];
		has_mask_t& blk_has = blks_has_[this->get_blk(coord)];

		Tile& t = values_[progress];
		if (!t.is_clear()) {
			// If the tile is currently already set, clear its
			// previous value:
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
			return PathDirection::Forward;
		}
		*/
		for (ord2_t try_i = t.next_try_index; try_i < O2; try_i++) {
			const ord2_t try_val = val_try_order_[this->get_row(coord)][try_i];
			const has_mask_t try_val_mask = has_mask_t(1) << try_val;
			if (!(t_has & try_val_mask)) {
				// If a valid value is found for this tile:
				row_has |= try_val_mask;
				col_has |= try_val_mask;
				blk_has |= try_val_mask;
				t.value = try_val;
				t.next_try_index = (try_i + 1u);
				return PathDirection::Forward;
			}
		}
		// Backtrack:
		// - turning back: The above loop never entered the return-block.
		// - continuing back: The above loop was completely skipped-over.
		t.clear();
		return PathDirection::Back;
	}
}