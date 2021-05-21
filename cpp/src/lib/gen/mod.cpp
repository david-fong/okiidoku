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
std::ostream& operator<<(std::ostream& os, solvent::lib::gen::Generator<O> const& s) {
	using namespace solvent::lib::gen;
	using length_t = typename solvent::size<O>::length_t;
	#define PRINT_GRID0_TILE(PRINTER_STATEMENT) {\
		for (length_t col = 0; col < s.O2; col++) {\
			if (isPretty && (col % s.O1) == 0) os << Ansi::DIM.ON << " |" << Ansi::DIM.OFF;\
			PRINTER_STATEMENT;\
		}}
	#define PRINT_GRID_TILE(PRINTER_STATEMENT) {\
		if (isPretty) {\
			os << Ansi::DIM.ON << " |" /* << Ansi::DIM.OFF */;\
			os << GRID_SEP;\
			PRINT_GRID0_TILE(PRINTER_STATEMENT)}\
		}

	const bool isPretty = &os == &std::cout;
	const auto printBlkRowSepString = [&](){
		if (!isPretty) return;
		os << '\n' << Ansi::DIM.ON;
		os << s.blkRowSepString;
		os << GRID_SEP << s.blkRowSepString;
		os << Ansi::DIM.OFF;
	};
	for (length_t row = 0; row < s.O2; row++) {
		if (row % s.O1 == 0) {
			printBlkRowSepString();
		}
		os << '\n';
		// Tile content:
		#define index (row * s.length + col)
		PRINT_GRID0_TILE(os << ' ' << s.grid[index])
		PRINT_GRID_TILE(s.printShadedBacktrackStat(s.backtrackCounts[index]))
		// PRINT_GRID_TILE(os << std::setw(2) << grid[index].biasIndex)
		// PRINT_GRID_TILE(os << ' ' << rowBiases[row][col])
		#undef index
		if (isPretty) os << Ansi::DIM.ON << " |" << Ansi::DIM.OFF;
	}
	printBlkRowSepString();
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
Path::E Generator<O>::genPath = initializePath();
template <Order O>
std::array<typename size<O>::area_t, Generator<O>::O4> Generator<O>::traversalOrder;
#endif


// Mechanism to statically toggle printing alignment:
// (#undef-ed before the end of this namespace)
#define STATW_I << std::setw(this->STATS_WIDTH)
#define STATW_D << std::setw(this->STATS_WIDTH + 4)


template <Order O>
Generator<O>::Generator(std::ostream& os):
	os        (os),
	isPretty  (&os == &std::cout)
{
	for (auto& rowBias : rowBiases) {
		std::iota(rowBias.begin(), rowBias.end(), 0);
	}
	// Note: This #if must be done here and not in `initializePath`
	// because that method is still needed for static initialization.
	#if !SOLVER_THREADS_SHARE_GENPATH
	initializePath();
	#endif

	// Output formatting:
	if (isPretty) {
		os.imbue(std::locale(os.getloc(), new MyNumpunct()));
	}
	os.precision(3);
	os << std::fixed;
}


template <Order O>
void Generator<O>::copy_settings_from(Generator const& other) {
	#if !SOLVER_THREADS_SHARE_GENPATH
	setPath(other.getPath());
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
void Generator<O>::printSimple(void) const {
	auto const helper = [this](std::ostream& os, const bool isPretty){
		const bool doDim = isPretty && (prev_gen.getExitStatus() != ExitStatus::Ok);
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
void Generator<O>::printShadedBacktrackStat(const backtrack_t count) const {
	const unsigned int relativeIntensity
		= (double)(count - 1)
		* Ansi::BLOCK_CHARS.size()
		/ max_backtracks;
	auto const& intensityChar
		= (count != 0)
		? Ansi::BLOCK_CHARS[relativeIntensity]
		: " ";
	os << intensityChar << intensityChar;
}


template <Order O>
void Generator<O>::clear(void) {
	for (area_t i = 0; i < O4; i++) {
		grid[i].clear();
	}
	rowSymbolOccMasks.fill(0);
	colSymbolOccMasks.fill(0);
	blkSymbolOccMasks.fill(0);
	backtrackCounts.fill(0u);
	max_backtracks = 0u;

	// Scramble each row's value-guessing-O1:
	RANDOM_MUTEX.lock();
	for (auto& rowBias : rowBiases) {
		std::shuffle(rowBias.begin(), rowBias.end(), VALUE_RNG);
	}
	RANDOM_MUTEX.unlock();
}


template <Order O>
void Generator<O>::generateSolution(const bool contPrev) {
	PathDirection direction = PathDirection::Forward;
	opcount_t    opCount   = 0u;
	area_t       tvsIndex  = 0u;

	if (__builtin_expect(contPrev, false)) {
		switch (prev_gen.getExitStatus()) {
			case ExitStatus::Ok:
				// Previously succeeded.
				tvsIndex  = O4 - 1u;
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
	prev_gen.exitStatus = ExitStatus::Ok; // default value;

	while (tvsIndex < O4) {
		const area_t gridIndex = traversalOrder[tvsIndex];
		// Try something at the current tile:
		direction = setNextValid(gridIndex);
		opCount++;
		if (direction == PathDirection::Back) {
			// Pop and step backward:
			if (++backtrackCounts[gridIndex] > max_backtracks) {
				max_backtracks = backtrackCounts[gridIndex];
			}
			if (__builtin_expect(tvsIndex == 0u, false)) {
				prev_gen.exitStatus = ExitStatus::Exhausted;
				break;
			}
			--tvsIndex;
		} else {
			// (direction == PathDirection::Forward)
			++tvsIndex;
		}
		// Check whether the give-up-condition has been met:
		if (__builtin_expect(max_backtracks >= GIVEUP_THRESHOLD, false)) {
			prev_gen.exitStatus = ExitStatus::Abort;
			break;
		}
	}
	prev_gen.tvsIndex = tvsIndex;
	prev_gen.opCount  = opCount;
}


template <Order O>
PathDirection Generator<O>::setNextValid(const area_t index) {
	occmask_t& rowBin = rowSymbolOccMasks[this->getRow(index)];
	occmask_t& colBin = colSymbolOccMasks[this->getCol(index)];
	occmask_t& blkBin = blkSymbolOccMasks[this->getBlk(index)];

	Tile& t = grid[index];
	if (!t.isClear()) {
		// If the tile is currently already set, clear its
		// previous value:
		const occmask_t eraseMask = ~(occmask_t(0b1u) << t.value);
		rowBin &= eraseMask;
		colBin &= eraseMask;
		blkBin &= eraseMask;
	}

	const occmask_t invalidBin = (rowBin | colBin | blkBin);
	// NOTE: these do not improve time-scaling performance, but I wish they did.
	/*
	if (__builtin_expect(occmask_popcount(invalidBin) == O2, false)) {
		t.clear();
		return Back;
	} else if (occmask_popcount(invalidBin) == O2 - 1) {
		const value_t value = occmask_ctz(!invalidBin);
		const occmask_t valueBit = 0b1 << value;
		rowBin |= valueBit;
		colBin |= valueBit;
		blkBin |= valueBit;
		t.value = value;
		t.biasIndex = O2;
		return Forward;
	}
	*/
	for (value_t biasIndex = t.biasIndex; biasIndex < O2; biasIndex++) {
		const value_t value = rowBiases[this->getRow(index)][biasIndex];
		const occmask_t valueBit = occmask_t(0b1u) << value;
		if (!(invalidBin & valueBit)) {
			// If a valid value is found for this tile:
			rowBin |= valueBit;
			colBin |= valueBit;
			blkBin |= valueBit;
			t.value = value;
			t.biasIndex = (biasIndex + 1u);
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
Path::E Generator<O>::setPath(const Path::E newPath, const bool force) noexcept {
	if (!force && newPath == getPath()) {
		// Short circuit:
		return getPath();
	}
	switch (newPath) {
		case Path::E::RowMajor:
			std::iota(traversalOrder.begin(), traversalOrder.end(), 0);
			break;
		case Path::E::DealRwMj: {
			area_t i = 0;
			for (order_t bRow = 0; bRow < O1; bRow++) {
				for (order_t bCol = 0; bCol < O1; bCol++) {
					for (length_t blk = 0; blk < O2; blk++) {
						area_t blkaddr = ((blk % O1) * O1) + (blk / O1 * O1 * O2);
						traversalOrder[i++] = blkaddr + (bRow * O2) + bCol;
					}
				}
			}
			break; }
		case Path::E::BlockCol: {
			area_t i = 0;
			for (order_t blkCol = 0; blkCol < O1; blkCol++) {
				for (length_t row = 0; row < O2; row++) {
					for (order_t bCol = 0; bCol < O1; bCol++) {
						traversalOrder[i++] = (blkCol * O1) + (row * O2) + (bCol);
					}
				}
			}
			break; }
	}
	const Path::E oldPath = getPath();
	genPath = newPath;
	return oldPath;
}


template <Order O>
Path::E Generator<O>::setPath(std::string const& newPathString) noexcept {
	std::cout << "\ngenerator path is ";
	if (newPathString.empty()) {
		std::cout << "currently set to: " << getPath() << std::endl;
		return getPath();
	}
	for (unsigned i = 0; i < Path::size; i++) {
		if (newPathString.compare(Path::NAMES[i]) == 0) {
			if (Path::E{i} == getPath()) {
				std::cout << "already set to: ";
			} else {
				std::cout << "now set to: ";
				setPath(Path::E{i});
			}
			std::cout << getPath() << std::endl;
			return getPath();
		}
	}
	// unsuccessful return:
	std::cout << getPath() << " (unchanged).\n"
		<< Ansi::RED.ON << '"' << newPathString
		<< "\" is not a valid generator path name.\n"
		<< Path::OPTIONS_MENU << Ansi::RED.OFF << std::endl;
	return getPath();
}


template <Order O>
Path::E Generator<O>::initializePath(void) noexcept {
	Path::E defaultPath;
	// Interesting: Smaller-O1 grids perform better with RowMajor as genPath.
	if (O1 <= 4) {
		defaultPath = Path::E::RowMajor;
	} else {
		defaultPath = Path::E::BlockCol;
	}
	setPath(defaultPath, true);
	return getPath();
}




template <Order O>
void Generator<O>::printMessageBar(
	std::string const& msg,
	unsigned barLength,
	const char fillChar
) const {
	if (barLength < msg.O2() + 8) {
		barLength = msg.O2() + 8;
	}
	std::string bar(barLength, fillChar);
	if (!msg.empty()) {
		bar.replace(4, msg.O2(), msg);
		bar.at(3) = ' ';
		bar.at(4 + msg.O2()) = ' ';
	}
	os << '\n' <<bar;
}


template <Order O>
void Generator<O>::printMessageBar(std::string const& msg, const char fillChar) const {
	const unsigned gridBarLength = (isPretty)
		? ((O2 + O1 + 1) * 2)
		: (O2 * 2);
	constexpr unsigned numGrids = 2u;
	unsigned allBarLength = (numGrids * gridBarLength);
	if (numGrids > 1) allBarLength += (numGrids - 1) * GRID_SEP.length();
	return printMessageBar(msg, allBarLength + 1, fillChar);
}

#undef STATW_I
#undef STATW_D


template <Order O>
const std::string Generator<O>::blkRowSepString = [](const unsigned order) {
	std::string vbar = " " + std::string((((order * (order + 1)) + 1) * 2 - 1), '-');
	for (unsigned i = 0; i <= order; i++) {
		// Insert crosses at vbar intersections.
		vbar[(2 * (order + 1) * i) + 1] = '+';
	}
	return vbar;
}(O);

}