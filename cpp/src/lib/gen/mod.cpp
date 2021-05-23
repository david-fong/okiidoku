#include "./mod.hpp"

#include "../../util/ansi.hpp"

#include <mutex>
#include <iostream>    // cout, endl
#include <iomanip>     // setbase, setw,
#include <fstream>     // ifstream,
#include <ctime>       // clock,
#include <numeric>     // iota,
#include <random>
#include <algorithm>   // random_shuffle,
#include <string>      // string,


template<solvent::Order O>
std::ostream& operator<<(std::ostream& os, solvent::lib::gen::Generator<O> const& g) {
	using namespace solvent::lib::gen;
	using ord2_t = typename solvent::size<O>::ord2_t;
	#define _M_PRINT_GRID0_TILE(PRINTER_STATEMENT) {\
		for (ord2_t col = 0; col < g.O2; col++) {\
			if (is_pretty && (col % g.O1) == 0) os << Ansi::DIM.ON << " |" << Ansi::DIM.OFF;\
			PRINTER_STATEMENT;\
		}}
	#define _M_PRINT_GRID_TILE(PRINTER_STATEMENT) {\
		if (is_pretty) {\
			os << Ansi::DIM.ON << " |" /* << Ansi::DIM.OFF */;\
			os << GRID_SEP;\
			_M_PRINT_GRID0_TILE(PRINTER_STATEMENT)}\
		}

	const bool is_pretty = &os == &std::cout;
	const auto print_blk_row_sep_str = [&](){
		if (!is_pretty) return;
		os << '\n' << Ansi::DIM.ON;
		os << g.blk_row_sep_str;
		os << GRID_SEP << g.blk_row_sep_str;
		os << Ansi::DIM.OFF;
	};
	for (ord2_t row = 0; row < g.O2; row++) {
		if (row % g.O1 == 0) {
			print_blk_row_sep_str();
		}
		os << '\n';
		// Tile content:
		#define _M_index ((row * g.O2) + col)
		_M_PRINT_GRID0_TILE(os << ' ' << g.pathy_values[_M_index])
		_M_PRINT_GRID_TILE(g.print_shaded_backtrack_stat(g.backtrack_counts[_M_index]))
		// _M_PRINT_GRID_TILE(os << std::setw(2) << pathy_values[coord].try_index)
		// _M_PRINT_GRID_TILE(os << ' ' << val_try_order[row][col])
		#undef _M_index
		if (is_pretty) os << Ansi::DIM.ON << " |" << Ansi::DIM.OFF;
	}
	print_blk_row_sep_str();
	#undef PRINT_GRID_TILE
	#undef PRINT_GRID0_TILE
	return os;
}


namespace solvent::lib::gen {

// Guards accesses to RMG. I currently only
// use this when shuffling generator biases.
std::mutex RANDOM_MUTEX;
std::mt19937 VALUE_RNG;

struct MyNumpunct : std::numpunct<char> {
	std::string do_grouping(void) const {
		return "\03";
	}
};

std::u8string shade_backtrack_stat(const long out_of, const long count) {
	const unsigned int relative_intensity
		= (double)(count - 1)
		* Ansi::BLOCK_CHARS.size()
		/ out_of;
	return  (count != 0)
		? Ansi::BLOCK_CHARS[relative_intensity]
		: u8" ";
}


// Mechanism to statically toggle printing alignment:
// (#undef-ed before the end of this namespace)
#define STATW_I << std::setw(this->STATS_WIDTH)
#define STATW_D << std::setw(this->STATS_WIDTH + 4)


template<Order O>
Generator<O>::Generator(std::ostream& os):
	os(os),
	is_pretty(&os == &std::cout)
{
	for (auto& row_bias : val_try_order) {
		std::iota(row_bias.begin(), row_bias.end(), 0);
	}

	// Output formatting:
	if (is_pretty) {
		os.imbue(std::locale(os.getloc(), new MyNumpunct()));
	}
	os.precision(3);
	os << std::fixed;
}


template<Order O>
void Generator<O>::copy_settings_from(Generator const& other) {
	set_path_kind(other.get_path_kind());
}


template<Order O>
void Generator<O>::print(void) const {
	if (&os != &std::cout) {
		std::cout << *this;
	}
	os << *this;
}


template<Order O>
void Generator<O>::print_simple(void) const {
	auto const helper = [this](std::ostream& os, const bool is_pretty){
		const bool do_dim = is_pretty && (generate_result.get_exit_status() != ExitStatus::Ok);
		if (do_dim) os << Ansi::DIM.ON;
		for (auto const& t : pathy_values) {
			os << t;
		}
		if (do_dim) os << Ansi::DIM.OFF;
	};
	if (&os == &std::cout) {
		helper(std::cout, true);
	} else {
		helper(std::cout, true);
		helper(os, false);
	}
}


template<Order O>
void Generator<O>::clear(void) {
	for (ord4_t i = 0; i < O4; i++) {
		pathy_values[i].clear();
	}
	rows_has.fill(0);
	cols_has.fill(0);
	blks_has.fill(0);
	backtrack_counts.fill(0u);

	// Scramble each row's value-guessing-O1:
	RANDOM_MUTEX.lock();
	for (auto& row_bias : val_try_order) {
		std::shuffle(row_bias.begin(), row_bias.end(), VALUE_RNG);
	}
	RANDOM_MUTEX.unlock();
}


template<Order O>
template<const PathKind::E PK>
GenResult Generator<O>::generate(const bool _continue) {
	auto direction = PathDirection::Forward;
	opcount_t op_count = 0;
	ord4_t progress = 0;
	backtrack_t most_backtracks = 0;

	if (_continue) [[unlikely]] {
		switch (generate_result.get_exit_status()) {
			case ExitStatus::Ok:
				progress  = O4 - 1u;
				direction = PathDirection::Back;
				break;
			case ExitStatus::Exhausted: return;
			default: break;
		}
	} else {
		// Not continuing.
		this->clear();
	}
	generate_result.exit_status = ExitStatus::Ok; // default value;

	while (progress < O4) {
		const ord4_t gridIndex = path[progress];
		// Try something at the current tile:
		direction = set_next_valid(gridIndex);
		op_count++;
		if (direction == PathDirection::Back) {
			// Pop and step backward:
			if (++backtrack_counts[gridIndex] > most_backtracks) {
				most_backtracks = backtrack_counts[gridIndex];
			}
			if (progress == 0) [[unlikely]] {
				generate_result.exit_status = ExitStatus::Exhausted;
				break;
			}
			--progress;
		} else {
			// (direction == PathDirection::Forward)
			++progress;
		}
		// Check whether the give-up-condition has been met:
		if (most_backtracks >= GIVEUP_THRESHOLD) [[unlikely]] {
			generate_result.exit_status = ExitStatus::Abort;
			break;
		}
	}
	generate_result.progress = progress;
	generate_result.op_count = op_count;
}


template<Order O>
PathDirection Generator<O>::set_next_valid(const ord4_t coord) {
	occmask_t& row_has = rows_has[this->get_row(coord)];
	occmask_t& col_has = cols_has[this->get_col(coord)];
	occmask_t& blk_has = blks_has[this->get_blk(coord)];

	Tile& t = pathy_values[coord];
	if (!t.is_clear()) {
		// If the tile is currently already set, clear its
		// previous value:
		const occmask_t erase_mask = ~(occmask_t(0b1u) << t.value);
		row_has &= erase_mask;
		col_has &= erase_mask;
		blk_has &= erase_mask;
	}

	const occmask_t invalid_bin = (row_has | col_has | blk_has);
	// NOTE: these do not improve time-scaling performance, but I wish they did.
	/*
	if (std::popcount(invalid_bin) == O2) [[unlikely]] {
		t.clear();
		return PathDirection::Back;
	} else if (std::popcount(invalid_bin) == O2 - 1) {
		const value_t value = std::countl_zero(!invalid_bin);
		const occmask_t value_bit = 0b1 << value;
		row_has |= value_bit;
		col_has |= value_bit;
		blk_has |= value_bit;
		t.value = value;
		t.try_index = O2;
		return PathDirection::Forward;
	}
	*/
	for (value_t try_index = t.try_index; try_index < O2; try_index++) {
		const value_t value = val_try_order[this->get_row(coord)][try_index];
		const occmask_t value_bit = occmask_t(1) << value;
		if (!(invalid_bin & value_bit)) {
			// If a valid value is found for this tile:
			row_has |= value_bit;
			col_has |= value_bit;
			blk_has |= value_bit;
			t.value = value;
			t.try_index = (try_index + 1u);
			return PathDirection::Forward;
		}
	}
	// Backtrack:
	// - turning back: The above loop never entered the return-block.
	// - continuing back: The above loop was completely skipped-over.
	t.clear();
	return PathDirection::Back;
}


template<Order O>
void Generator<O>::print_msg_bar(
	std::string const& msg,
	unsigned bar_length,
	const char fill_char
) const {
	if (bar_length < msg.length() + 8) {
		bar_length = msg.length() + 8;
	}
	std::string bar(bar_length, fill_char);
	if (!msg.empty()) {
		bar.replace(4, msg.length(), msg);
		bar.at(3) = ' ';
		bar.at(4 + msg.length()) = ' ';
	}
	os << '\n' <<bar;
}


template<Order O>
void Generator<O>::print_msg_bar(std::string const& msg, const char fill_char) const {
	const unsigned grid_bar_length = (is_pretty)
		? ((O2 + O1 + 1) * 2)
		: (O2 * 2);
	constexpr unsigned num_grids = 2u;
	unsigned all_bar_length = (num_grids * grid_bar_length);
	if (num_grids > 1) all_bar_length += (num_grids - 1) * GRID_SEP.length();
	return print_msg_bar(msg, all_bar_length + 1, fill_char);
}

#undef STATW_I
#undef STATW_D


template<Order O>
const std::string Generator<O>::blk_row_sep_str = [](const unsigned order) {
	std::string vbar = " " + std::string((((order * (order + 1)) + 1) * 2 - 1), '-');
	for (unsigned i = 0; i <= order; i++) {
		// Insert crosses at vbar intersections.
		vbar[(2 * (order + 1) * i) + 1] = '+';
	}
	return vbar;
}(O);

}