#include "./mod.hpp"

#include <mutex>
#include <iostream>    // cout, endl
#include <iomanip>     // setbase, setw,
#include <fstream>     // ifstream,
#include <ctime>       // clock,
#include <numeric>     // iota,
#include <random>
#include <algorithm>   // random_shuffle,
#include <string>      // string,


template <solvent::Order O>
std::ostream& operator<<(std::ostream& os, solvent::lib::gen::Generator<O> const& g) {
	using namespace solvent::lib::gen;
	using ord2_t = typename solvent::size<O>::ord2_t;
	#define PRINT_GRID0_TILE(PRINTER_STATEMENT) {\
		for (ord2_t col = 0; col < g.O2; col++) {\
			if (is_pretty && (col % g.O1) == 0) os << Ansi::DIM.ON << " |" << Ansi::DIM.OFF;\
			PRINTER_STATEMENT;\
		}}
	#define PRINT_GRID_TILE(PRINTER_STATEMENT) {\
		if (is_pretty) {\
			os << Ansi::DIM.ON << " |" /* << Ansi::DIM.OFF */;\
			os << GRID_SEP;\
			PRINT_GRID0_TILE(PRINTER_STATEMENT)}\
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
		#define index ((row * g.O2) + col)
		PRINT_GRID0_TILE(os << ' ' << g.grid[index])
		PRINT_GRID_TILE(g.print_shaded_backtrack_stat(g.backtrack_counts[index]))
		// PRINT_GRID_TILE(os << std::setw(2) << grid[index].try_progress)
		// PRINT_GRID_TILE(os << ' ' << val_try_order[row][col])
		#undef index
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

#if SOLVER_THREADS_SHARE_GENPATH
template <Order O>
Path::E Generator<O>::path_kind = initialize_path();
template <Order O>
std::array<typename size<O>::ord4_t, Generator<O>::O4> Generator<O>::path;
#endif


// Mechanism to statically toggle printing alignment:
// (#undef-ed before the end of this namespace)
#define STATW_I << std::setw(this->STATS_WIDTH)
#define STATW_D << std::setw(this->STATS_WIDTH + 4)


template <Order O>
Generator<O>::Generator(std::ostream& os):
	os        (os),
	is_pretty  (&os == &std::cout)
{
	for (auto& rowBias : val_try_order) {
		std::iota(rowBias.begin(), rowBias.end(), 0);
	}
	// Note: This #if must be done here and not in `initialize_path`
	// because that method is still needed for static initialization.
	#if !SOLVER_THREADS_SHARE_GENPATH
	initialize_path();
	#endif

	// Output formatting:
	if (is_pretty) {
		os.imbue(std::locale(os.getloc(), new MyNumpunct()));
	}
	os.precision(3);
	os << std::fixed;
}


template <Order O>
void Generator<O>::copy_settings_from(Generator const& other) {
	#if !SOLVER_THREADS_SHARE_GENPATH
	set_path_kind(other.get_path_kind());
	#endif
}


template <Order O>
void Generator<O>::print(void) const {
	if (&os != &std::cout) {
		std::cout << *this;
	}
	os << *this;
}


template <Order O>
void Generator<O>::print_simple(void) const {
	auto const helper = [this](std::ostream& os, const bool is_pretty){
		const bool doDim = is_pretty && (prev_gen.get_exist_status() != ExitStatus::Ok);
		if (doDim) os << Ansi::DIM.ON;
		for (auto const& t : grid) {
			os << t;
		}
		if (doDim) os << Ansi::DIM.OFF;
	};
	if (&os == &std::cout) {
		helper(std::cout, true);
	} else {
		helper(std::cout, true);
		helper(os, false);
	}
}


template <Order O>
void Generator<O>::print_shaded_backtrack_stat(const backtrack_t count) const {
	const unsigned int relativeIntensity
		= (double)(count - 1)
		* Ansi::BLOCK_CHARS.size()
		/ most_backtracks;
	auto const& intensityChar
		= (count != 0)
		? Ansi::BLOCK_CHARS[relativeIntensity]
		: " ";
	os << intensityChar << intensityChar;
}


template <Order O>
void Generator<O>::clear(void) {
	for (ord4_t i = 0; i < O4; i++) {
		grid[i].clear();
	}
	row_has.fill(0);
	col_has.fill(0);
	blk_has.fill(0);
	backtrack_counts.fill(0u);
	most_backtracks = 0u;

	// Scramble each row's value-guessing-O1:
	RANDOM_MUTEX.lock();
	for (auto& rowBias : val_try_order) {
		std::shuffle(rowBias.begin(), rowBias.end(), VALUE_RNG);
	}
	RANDOM_MUTEX.unlock();
}


template <Order O>
void Generator<O>::generate(const bool _continue) {
	PathDirection direction = PathDirection::Forward;
	opcount_t    op_count   = 0u;
	ord4_t       progress   = 0u;

	if (__builtin_expect(_continue, false)) {
		switch (prev_gen.get_exist_status()) {
			case ExitStatus::Ok:
				// Previously succeeded.
				progress  = O4 - 1u;
				direction = PathDirection::Back;
				break;
			case ExitStatus::Exhausted:
				return;
			default: break;
		}
	} else {
		// Not continuing. Do something entirely new!
		this->clear();
	}
	prev_gen.exit_status = ExitStatus::Ok; // default value;

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
			if (__builtin_expect(progress == 0u, false)) {
				prev_gen.exit_status = ExitStatus::Exhausted;
				break;
			}
			--progress;
		} else {
			// (direction == PathDirection::Forward)
			++progress;
		}
		// Check whether the give-up-condition has been met:
		if (__builtin_expect(most_backtracks >= GIVEUP_THRESHOLD, false)) {
			prev_gen.exit_status = ExitStatus::Abort;
			break;
		}
	}
	prev_gen.progress = progress;
	prev_gen.op_count  = op_count;
}


template <Order O>
PathDirection Generator<O>::set_next_valid(const ord4_t index) {
	occmask_t& rowBin = row_has[this->get_row(index)];
	occmask_t& colBin = col_has[this->get_col(index)];
	occmask_t& blkBin = blk_has[this->get_blk(index)];

	Tile& t = grid[index];
	if (!t.is_clear()) {
		// If the tile is currently already set, clear its
		// previous value:
		const occmask_t erase_mask = ~(occmask_t(0b1u) << t.value);
		rowBin &= erase_mask;
		colBin &= erase_mask;
		blkBin &= erase_mask;
	}

	const occmask_t invalidBin = (rowBin | colBin | blkBin);
	// NOTE: these do not improve time-scaling performance, but I wish they did.
	/*
	if (__builtin_expect(occmask_popcount(invalidBin) == O2, false)) {
		t.clear();
		return Back;
	} else if (occmask_popcount(invalidBin) == O2 - 1) {
		const value_t value = occmask_ctz(!invalidBin);
		const occmask_t value_bit = 0b1 << value;
		rowBin |= value_bit;
		colBin |= value_bit;
		blkBin |= value_bit;
		t.value = value;
		t.try_progress = O2;
		return Forward;
	}
	*/
	for (value_t try_progress = t.try_progress; try_progress < O2; try_progress++) {
		const value_t value = val_try_order[this->get_row(index)][try_progress];
		const occmask_t value_bit = occmask_t(0b1u) << value;
		if (!(invalidBin & value_bit)) {
			// If a valid value is found for this tile:
			rowBin |= value_bit;
			colBin |= value_bit;
			blkBin |= value_bit;
			t.value = value;
			t.try_progress = (try_progress + 1u);
			return PathDirection::Forward;
		}
	}
	// Backtrack:
	// - turning back: The above loop never entered the return-block.
	// - continuing back: The above loop was completely skipped-over.
	t.clear();
	return PathDirection::Back;
}


template <Order O>
Path::E Generator<O>::set_path_kind(const Path::E newPath, const bool force) noexcept {
	if (!force && newPath == get_path_kind()) {
		// Short circuit:
		return get_path_kind();
	}
	switch (newPath) {
		case Path::E::RowMajor:
			std::iota(path.begin(), path.end(), 0);
			break;
		case Path::E::DealRwMj: {
			ord4_t i = 0;
			for (ord1_t b_row = 0; b_row < O1; b_row++) {
				for (ord1_t b_col = 0; b_col < O1; b_col++) {
					for (ord2_t blk = 0; blk < O2; blk++) {
						ord4_t blkaddr = ((blk % O1) * O1) + (blk / O1 * O1 * O2);
						path[i++] = blkaddr + (b_row * O2) + b_col;
					}
				}
			}
			break; }
		case Path::E::BlockCol: {
			ord4_t i = 0;
			for (ord1_t blkCol = 0; blkCol < O1; blkCol++) {
				for (ord2_t row = 0; row < O2; row++) {
					for (ord1_t b_col = 0; b_col < O1; b_col++) {
						path[i++] = (blkCol * O1) + (row * O2) + (b_col);
					}
				}
			}
			break; }
	}
	const Path::E oldPath = get_path_kind();
	path_kind = newPath;
	return oldPath;
}


template <Order O>
Path::E Generator<O>::set_path_kind(std::string const& newPathString) noexcept {
	std::cout << "\ngenerator path is ";
	if (newPathString.empty()) {
		std::cout << "currently set to: " << get_path_kind() << std::endl;
		return get_path_kind();
	}
	for (unsigned i = 0; i < Path::size; i++) {
		if (newPathString.compare(Path::NAMES[i]) == 0) {
			if (Path::E{i} == get_path_kind()) {
				std::cout << "already set to: ";
			} else {
				std::cout << "now set to: ";
				set_path_kind(Path::E{i});
			}
			std::cout << get_path_kind() << std::endl;
			return get_path_kind();
		}
	}
	// unsuccessful return:
	std::cout << get_path_kind() << " (unchanged).\n"
		<< Ansi::RED.ON << '"' << newPathString
		<< "\" is not a valid generator path name.\n"
		<< Path::OPTIONS_MENU << Ansi::RED.OFF << std::endl;
	return get_path_kind();
}


template <Order O>
Path::E Generator<O>::initialize_path(void) noexcept {
	Path::E _default;
	// Interesting: Smaller-O1 grids perform better with RowMajor as path_kind.
	if (O1 <= 4) {
		_default = Path::E::RowMajor;
	} else {
		_default = Path::E::BlockCol;
	}
	set_path_kind(_default, true);
	return get_path_kind();
}




template <Order O>
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


template <Order O>
void Generator<O>::print_msg_bar(std::string const& msg, const char fill_char) const {
	const unsigned gridBarLength = (is_pretty)
		? ((O2 + O1 + 1) * 2)
		: (O2 * 2);
	constexpr unsigned numGrids = 2u;
	unsigned allBarLength = (numGrids * gridBarLength);
	if (numGrids > 1) allBarLength += (numGrids - 1) * GRID_SEP.length();
	return print_msg_bar(msg, allBarLength + 1, fill_char);
}

#undef STATW_I
#undef STATW_D


template <Order O>
const std::string Generator<O>::blk_row_sep_str = [](const unsigned order) {
	std::string vbar = " " + std::string((((order * (order + 1)) + 1) * 2 - 1), '-');
	for (unsigned i = 0; i <= order; i++) {
		// Insert crosses at vbar intersections.
		vbar[(2 * (order + 1) * i) + 1] = '+';
	}
	return vbar;
}(O);

}