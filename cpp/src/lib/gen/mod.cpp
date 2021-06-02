#include "./mod.hpp"
#include ":/util/ansi.hpp"

#include <iostream>
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
		backtracks_.fill(0);

		RNG_MUTEX.lock();
		for (auto& vto : val_try_orders_) {
			std::shuffle(vto.begin(), vto.end(), Rng);
		}
		RNG_MUTEX.unlock();
	}


	template<Order O>
	Generator<O>::GenResult Generator<O>::generate(const std::optional<Params> params) {
		GenResult info;
		if (params) [[likely]] {
			info.params = params.value();
			if (info.params.max_backtracks == 0) {
				info.params.max_backtracks = DEFAULT_MAX_BACKTRACKS;
			}
			this->init();
		} else {
			// Continue the previous generation.
			info = gen_result_;
			if (info.status == ExitStatus::Exhausted) [[unlikely]] {
				return info;
			}
			backtracks_.fill(0);
		}
		ord4_t (& prog2coord)(ord4_t) = path::PathCoords<O>[info.params.path_kind];
		ord4_t dead_end_progress = info.progress;

		while (true) {
			const bool do_backtrack = this->set_next_valid(
				info.progress, dead_end_progress, prog2coord
			);
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
				if (info.progress == O4-1) [[unlikely]] {
					info.status = ExitStatus::Ok;
					break;
				}
				++info.progress;
				if (!can_coords_see_each_other(prog2coord(info.progress), prog2coord(dead_end_progress))
					|| (info.progress > dead_end_progress)
				) {
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
	bool Generator<O>::set_next_valid(
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
		if ((progress < dead_end_progress) && !can_coords_see_each_other(
			prog2coord(progress), prog2coord(dead_end_progress)
		)) {
			t.clear();
			return true;
		}

		const has_mask_t t_has = (row_has | col_has | blk_has);
		for (ord2_t try_i = t.next_try_index; try_i < O2; try_i++) {
			const ord2_t try_val = val_try_orders_[progress/O2][try_i];
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
		// Nothing left to try here. Backtrack:
		t.clear();
		return true;
	}


	std::string shaded_backtrack_stat(const long out_of, const long count) {
		const unsigned int relative_intensity
			= static_cast<double>(count - 1)
			* util::ansi::BLOCK_CHARS.size()
			/ out_of;
		return (count == 0) ? " " : util::ansi::BLOCK_CHARS[relative_intensity];
	}


	template<solvent::Order O>
	std::ostream& operator<<(std::ostream& os, solvent::lib::gen::Generator<O> const& g) {
		using namespace solvent::lib::gen;
		namespace ansi = solvent::util::ansi;
		using ord2_t = typename solvent::size<O>::ord2_t;
		static const std::string GRID_SEP = "   ";

		static const auto blk_row_sep_str = []() {
			std::string vbar = " " + std::string((((O * (O + 1)) + 1) * 2 - 1), '-');
			for (unsigned i = 0; i <= O; i++) {
				vbar[(2 * (O + 1) * i) + 1] = '+';
			}
			return vbar;
		}();

		#define M_PRINT_GRID0_TILE(PRINTER_STATEMENT) {\
			for (ord2_t col = 0; col < g.O2; col++) {\
				if (is_pretty && (col % g.O1) == 0) os << ansi::DIM.ON << " |" << ansi::DIM.OFF;\
				PRINTER_STATEMENT;\
			}\
		}
		#define M_PRINT_GRID_TILE(PRINTER_STATEMENT) {\
			if (is_pretty) {\
				os << ansi::DIM.ON << " |";/* << ansi::DIM.OFF */\
				os << GRID_SEP;\
				M_PRINT_GRID0_TILE(PRINTER_STATEMENT)}\
			}

		const bool is_pretty = &os == &std::cout;
		const auto print_blk_row_sep_str = [&](){
			if (!is_pretty) return;
			os << '\n' << ansi::DIM.ON;
			os << g.blk_row_sep_str;
			os << GRID_SEP << g.blk_row_sep_str;
			os << ansi::DIM.OFF;
		};
		for (ord2_t row = 0; row < g.O2; row++) {
			if (row % g.O1 == 0) {
				print_blk_row_sep_str();
			}
			os << '\n';
			// Tile content:
			#define _M_index ((row * g.O2) + col)
			M_PRINT_GRID0_TILE(os << ' ' << g.values_[_M_index])
			M_PRINT_GRID_TILE(g.print_shaded_backtrack_stat(g.backtracks_[_M_index]))
			// M_PRINT_GRID_TILE(os << std::setw(2) << values_[coord].next_try_index)
			// M_PRINT_GRID_TILE(os << ' ' << val_try_order_[row][col])
			#undef _M_index
			if (is_pretty) os << ansi::DIM.ON << " |" << ansi::DIM.OFF;
		}
		print_blk_row_sep_str();
		#undef M_PRINT_GRID_TILE
		#undef M_PRINT_GRID0_TILE
		return os;
	}


	template<Order O>
	void Generator<O>::print_simple(std::ostream& os) const {
		auto const helper = [this](std::ostream& os, const bool is_pretty) -> void {
			const bool do_dim = is_pretty && (gen_result_.get_exit_status() != ExitStatus::Ok);
			if (do_dim) os << util::ansi::DIM.ON;
			for (auto const& t : values_) {
				os << t;
			}
			if (do_dim) os << util::ansi::DIM.OFF;
		};
		if (&os == &std::cout) {
			helper(std::cout, true);
		} else {
			helper(std::cout, true);
			helper(os, false);
		}
	}
}