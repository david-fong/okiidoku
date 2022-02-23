#include <solvent_lib/gen/mod.hpp>
#include <solvent_lib/morph/canon.hpp>
#include <solvent_lib/print.hpp>
#include <solvent_util/str.hpp>

#include <iostream>
#include <string>
#include <mutex>
#include <algorithm>   // shuffle,
#include <random>

namespace solvent::lib::gen {

	// long long total = 0;
	// long long true_ = 0;

	std::mt19937 Rng_;
	void seed_rng(std::uint_fast64_t seed) noexcept {
		Rng_.seed(seed);
	}
	// Guards accesses to Rng_. Only used when shuffling generator biases.
	std::mutex RNG_MUTEX;


	Params Params::clean(const Order O) noexcept {
		if (max_dead_ends == 0) {
			max_dead_ends = DEFAULT_MAX_DEAD_ENDS(O);
		}
		return *this;
	}


	template<Order O>
	ExitStatus Generator<O>::get_exit_status(void) const noexcept {
		switch (progress_) {
			case O4-1: return ExitStatus::Ok;
			case O4:   return ExitStatus::Exhausted;
			default:   return ExitStatus::Abort;
		}
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
		if (this->get_exit_status() == ExitStatus::Exhausted) [[unlikely]] {
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
		backtrack_origin_ = 0;
		most_dead_ends_seen_ = 0;
		op_count_ = 0;

		RNG_MUTEX.lock();
		for (auto& vto : val_try_orders_) {
			std::shuffle(vto.begin(), vto.end(), Rng_);
		}
		RNG_MUTEX.unlock();
	}


	template<Order O>
	void Generator<O>::generate_() {
		typename path::coord_converter_t<O> prog2coord = path::GetPathCoords<O>(params_.path_kind);

		bool backtracked = op_count_ != 0;
		while (true) [[likely]] {
			const Direction direction = this->set_next_valid_(prog2coord, backtracked);
			backtracked = direction.is_back;
			if (!direction.is_back_skip) [[likely]] { ++op_count_; }

			if (direction.is_back) [[unlikely]] {
				if (progress_ == 0) [[unlikely]] {
					break;
				}
				if (!direction.is_back_skip) [[unlikely]] {
					const dead_ends_t dead_ends = ++dead_ends_[progress_];
					if (dead_ends > most_dead_ends_seen_) [[unlikely]] {
						most_dead_ends_seen_ = dead_ends;
						if (dead_ends > params_.max_dead_ends) [[unlikely]] {
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
	}


	template<Order O>
	Direction Generator<O>::set_next_valid_(typename path::coord_converter_t<O> prog2coord, const bool backtracked) noexcept {
		const ord4x_t coord = prog2coord(progress_);
		has_mask_t& row_has = rows_has_[rmi2row<O>(coord)];
		has_mask_t& col_has = cols_has_[rmi2col<O>(coord)];
		has_mask_t& blk_has = blks_has_[rmi2blk<O>(coord)];
		const auto& val_try_order = val_try_orders_[progress_ / O2];

		Cell& cell = cells_[progress_];
		if (backtracked) [[unlikely]]/* average direction is forward */ {
			// Clear the current value from all masks:
			const has_mask_t erase_mask = ~( has_mask_t(0b1u) << val_try_order[cell.try_index] );
			row_has &= erase_mask;
			col_has &= erase_mask;
			blk_has &= erase_mask;

			// Smart skip-backtracking:
			// This optimization's degree of usefulness depends on the genpath and size.
			if (!cells_share_house<O>(coord, prog2coord(backtrack_origin_))) [[unlikely]] {
				// only likely when dealrwmj. dealrwmj is so slow already it doesn't
				// feel worth slowing other genpaths down to microoptimize.
				cell.clear();
				return Direction { .is_back = true, .is_back_skip = true };
			}
		}

		const has_mask_t cell_has = (row_has | col_has | blk_has);
		if (std::popcount(cell_has) != O2) [[likely]] {
			// The above optimization comes into effect ~1/5 of the time for size 5.
			for (ord2i_t try_i = static_cast<ord2i_t>((cell.try_index+1u) % (O2+1)); try_i < O2; try_i++) [[likely]] {
				const has_mask_t try_val_mask = has_mask_t{1} << val_try_order[try_i];
				if (!(cell_has & try_val_mask)) [[unlikely]] {
					// A valid value was found:
					row_has |= try_val_mask;
					col_has |= try_val_mask;
					blk_has |= try_val_mask;
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


	template<Order O>
	GenResult Generator<O>::make_gen_result_(void) const {
		typename path::coord_converter_t<O> prog2coord = path::GetPathCoords<O>(params_.path_kind);
		using GR = GenResult;
		GenResult gen_result {
			.O {O},
			.params {params_},
			.status {this->get_exit_status()},
			.backtrack_origin {static_cast<GR::backtrack_origin_t>(backtrack_origin_)}, // safe narrowing
			.most_dead_ends_seen {static_cast<GR::dead_ends_t>(most_dead_ends_seen_)},
			.op_count {op_count_},
			.grid = std::vector<GR::val_t>(O4, O2),
			.dead_ends = std::vector<GR::dead_ends_t>(O4, 0),
		};
		for (ord4i_t p = 0; p < O4; p++) {
			// Note: The bound under progress_ is significant.
			// try_indexes afterward are out of val_try_order's range.
			const ord4x_t coord = prog2coord(p);
			if (!cells_[p].is_clear()) {
				gen_result.grid[coord] = val_try_orders_[p/O2][cells_[p].try_index];
			}
			gen_result.dead_ends[coord] = static_cast<GR::dead_ends_t>(dead_ends_[p]);
		}
		if (params_.canonicalize && gen_result.status == ExitStatus::Ok) /* [[unlikely]] (worth?) */ {
			gen_result.grid = morph::canonicalize<O>(gen_result.grid_const_span<O>());
		}
		return gen_result;
	}


	std::string shaded_dead_end_stat(GenResult::dead_ends_t out_of, GenResult::dead_ends_t count) {
		assert(count <= out_of);
		return (count == 0) ? " " : util::str::BLOCK_CHARS[static_cast<std::size_t>(
			(count - 1) * util::str::BLOCK_CHARS.size() / (out_of + 1)
		)];
	}


	void GenResult::print_text(std::ostream& os) const {
		print::serial(os, O, [this](uint32_t coord) { return this->grid[coord]; });
	}


	void GenResult::print_pretty(std::ostream& os) const {
		const std::vector<print::print_grid_t> grid_accessors {
			print::print_grid_t([this](std::ostream& _os, uint16_t coord) {
				_os << ' '; print::val2str(_os, O, this->grid[coord]);
			}),
			print::print_grid_t([this](std::ostream& _os, uint16_t coord) {
				const auto shade = shaded_dead_end_stat(static_cast<dead_ends_t>(params.max_dead_ends), dead_ends[coord]);
				_os << shade << shade;
			}),
		};
		print::pretty(os, O, grid_accessors);
	}


	#define M_SOLVENT_TEMPL_TEMPL(O_) \
		template class Generator<O_>;
	M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef M_SOLVENT_TEMPL_TEMPL
}