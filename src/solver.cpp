#include "solver.hpp"

#include <iostream>     // cout, endl
#include <iomanip>      // setbase, setw,
#include <fstream>      // ifstream,
#include <ctime>        // clock,
#include <numeric>      // iota,
#include <algorithm>    // random_shuffle,
#include <string>       // string,

[[gnu::const]] static std::string createHSepString(unsigned int order);


/**
 * 
 */
namespace Sudoku {

// Mechanism to statically toggle printing alignment:
// (#undef-ed before the end of this namespace)
#define STATW_I << std::setw(this->statsWidth)
#define STATW_D << std::setw(this->statsWidth + 4)


template <Order O, bool CBT>
Solver<O,CBT>::Solver(std::ostream& os):
    os          (os),
    isPretty    (&os == &std::cout),
    blkRowSepString(createHSepString(order))
{
    for (auto& rowBias : rowBiases) {
        std::iota(rowBias.begin(), rowBias.end(), 0);
    }
    // Interesting: Smaller-order grids perform better with ROW_MAJOR as genPath.
    setGenPath((O < 4) ? ROW_MAJOR : BLOCK_COLS);

    // Output formatting:
    if (isPretty) {
        benchedLocale = os.imbue(std::locale(os.getloc(), new MyNumpunct()));
    }
    os.precision(3);
    os << std::fixed;
}


template <Order O, bool CBT>
void Solver<O,CBT>::print(void) const {
    #define PRINT_GRID0_TILE(PRINTER_STATEMENT) {\
        for (length_t col = 0; col < length; col++) {\
            if (isPretty && (col % order) == 0) os << " |";\
            PRINTER_STATEMENT;\
        }}
    #define PRINT_GRID_TILE(PRINTER_STATEMENT) {\
        if (isPretty) os << " |";\
        os << GRID_SEP;\
        PRINT_GRID0_TILE(PRINTER_STATEMENT)}

    if constexpr (CBT) {
        os << "max backtracks: " STATW_I << maxBacktrackCount << '\n';
    }
    if constexpr (order == 4) os << std::setbase(16);

    for (length_t row = 0; row < length; row++) {
        if (isPretty && (row % order == 0)) {
            // Print block-row separator string:
            os << blkRowSepString;
            if constexpr (CBT) os << GRID_SEP << blkRowSepString;
            os << '\n';
        }
        // Tile content:
        PRINT_GRID0_TILE(os << ' ' << grid[row * length + col])
        if constexpr (CBT) {
            PRINT_GRID_TILE(printShadedBacktrackStat(backtrackCounts[row * length + col]))
        }
        // PRINT_GRID_TILE(os << std::setw(2) << grid[row * length + col].biasIndex)
        // PRINT_GRID_TILE(os << ' ' << rowBiases[row][col])
        if (isPretty) os << " |";
        os << '\n';
    }
    if (isPretty) {
        os << blkRowSepString;
        if constexpr (CBT) os << GRID_SEP << blkRowSepString;
        os << '\n';
    }
    if constexpr (order == 4) os << std::setbase(10);
    os << std::endl;
}


template <Order O, bool CBT>
void Solver<O,CBT>::printShadedBacktrackStat(const unsigned count) const {
    static const std::array<std::string, 4> GREYSCALE_BLOCK_CHARS = {
        // NOTE: Make sure that the initializer list size matches that
        // of the corresponding template argument. Compilers won't warn.
        // See https://cppreference.com/w/cpp/language/sizeof...#Example
        // for an example utility function I can make to avoid this problem.
        u8"\u2591", u8"\u2592", u8"\u2593", u8"\u2588",
    };

    if constexpr (CBT) {
        const unsigned int relativeIntensity
            = (double)(count - 1)
            * GREYSCALE_BLOCK_CHARS.size()
            / maxBacktrackCount;
        auto const& intensityChar
            = (count != 0)
            ? GREYSCALE_BLOCK_CHARS[relativeIntensity]
            : " ";
        os << intensityChar << intensityChar;
    } else {
        // Complain if found in call-path when backtrack-counting is off:
        static_assert(CBT, "not avaliable when backtrack-counting is off.");
    }
}


template <Order O, bool CBT>
void Solver<O,CBT>::clear(void) {
    std::for_each(grid.begin(), grid.end(), [](Tile& t){ t.clear(); });
    rowSymbolOccMasks.fill(0);
    colSymbolOccMasks.fill(0);
    blkSymbolOccMasks.fill(0);
    if constexpr (CBT) backtrackCounts.fill(0);

    // Scramble each row's value-guessing-order:
    for (auto& rowBias : rowBiases) {
        std::random_shuffle(rowBias.begin(), rowBias.end(), MY_RANDOM);
    }
    // Do not clear seeds here. That can be done when reading in givens.
}




template <Order O, bool CBT>
template <bool USE_PUZZLE>
opcount_t Solver<O,CBT>::generateSolution(void) {
    opcount_t numOperations = 0;
    area_t tvsIndex = 0; // traversal index.

    while (tvsIndex < area) {
        const auto gridIndex = traversalOrder[tvsIndex];
        if (setNextValid(gridIndex) == TraversalDirection::BACK) {
            // Pop and step backward:
            if (__builtin_expect(tvsIndex == 0, false)) {
                // No solution could be found. Treat as if giveup:
                totalGenCount++;
                return 0;
            }
            if constexpr (CBT) {
                if (++backtrackCounts[gridIndex] > maxBacktrackCount) {
                    maxBacktrackCount = backtrackCounts[gridIndex];
                }
            }
            if constexpr (!USE_PUZZLE) {
                --tvsIndex;
            } else {
                // Backtrack, skipping over tiles containing given information:
                while (__builtin_expect(isTileForGiven[traversalOrder[--tvsIndex]], false)) {
                    if (__builtin_expect(tvsIndex == 0, false)) {
                        // No solution could be found. Treat as if giveup:
                        totalGenCount++;
                        return 0;
                    }
                }
            }
        } else {
            if constexpr (!USE_PUZZLE) {
                ++tvsIndex;
            } else {
                while (__builtin_expect(
                    (++tvsIndex < area) && isTileForGiven[traversalOrder[tvsIndex]],
                false));
            }
        }

        numOperations++;
        if (__builtin_expect(numOperations > GIVEUP_THRESHOLD, false)) {
            // Giveup threshold has been exceeded:
            totalGenCount++;
            return 0;
        }
    }
    totalGenCount++;
    return numOperations;
}


template <Order O, bool CBT>
TraversalDirection Solver<O,CBT>::setNextValid(const area_t index) {
    occmask_t& rowBin = rowSymbolOccMasks[getRow(index)];
    occmask_t& colBin = colSymbolOccMasks[getCol(index)];
    occmask_t& blkBin = blkSymbolOccMasks[getBlk(index)];

    Tile& t = grid[index];
    if (!t.isClear()) {
        // If the tile is currently already set, clear it:
        // NOTE: this is the same as "if backtracked to here".
        const occmask_t eraseMask = ~(0b1 << t.value);
        rowBin &= eraseMask;
        colBin &= eraseMask;
        blkBin &= eraseMask;
    }

    const occmask_t invalidBin = (rowBin | colBin | blkBin);
    // NOTE: these do not improve time-scaling performance, but I wish they did.
    /*
    if (__builtin_expect(occmask_popcount(invalidBin) == length, false)) {
        t.clear();
        return BACK;
    }
    else if (occmask_popcount(invalidBin) == length - 1) {
        const value_t value = occmask_ctz(!invalidBin);
        const occmask_t valueBit = 0b1 << value;
        rowBin |= valueBit;
        colBin |= valueBit;
        blkBin |= valueBit;
        t.value = value;
        t.biasIndex = length;
        return FORWARD;
    }
    */
    for (value_t biasIndex = t.biasIndex; biasIndex < length; biasIndex++) {
        const value_t value = rowBiases[getRow(index)][biasIndex];
        const occmask_t valueBit = 0b1 << value;
        if (!(invalidBin & valueBit)) {
            // If a valid value is found for this tile:
            rowBin |= valueBit;
            colBin |= valueBit;
            blkBin |= valueBit;
            t.value = value;
            t.biasIndex = (biasIndex + 1);
            return FORWARD;
        }
    }
    // Backtrack:
    t.clear();
    return BACK;
}


template <Order O, bool CBT>
void Solver<O,CBT>::setGenPath(const GenPath newGenPath) noexcept {
    switch (newGenPath) {
        case ROW_MAJOR:
            std::iota(traversalOrder.begin(), traversalOrder.end(), 0);
            break;
        case BLOCK_COLS: {
            area_t i = 0;
            for (order_t blkCol = 0; blkCol < order; blkCol++) {
                for (length_t row = 0; row < length; row++) {
                    for (order_t bCol = 0; bCol < order; bCol++) {
                        traversalOrder[i++] = (blkCol * order) + (row * length) + (bCol);
                    }
                }
            }
            break; }
    }
    genPath = newGenPath;
}








template <Order O, bool CBT>
void Solver<O,CBT>::printMessageBar(
    std::string const& msg,
    unsigned int barLength,
    const char fillChar
) const {
    if (barLength < msg.length() + 8) {
        barLength = msg.length() + 8;
    }
    std::string bar(barLength, fillChar);
    if (!msg.empty()) {
        bar.replace(4, msg.length(), msg);
        bar.at(3) = ' ';
        bar.at(4 + msg.length()) = ' ';
    }
    os << bar << '\n';
}


template <Order O, bool CBT>
void Solver<O,CBT>::printMessageBar(std::string const& msg, const char fillChar) const {
    // NOTE: If isPretty is change to be non-const, this cannot be static.
    static const unsigned int gridBarLength = (isPretty)
        ? ((length + order + 1) * 2)
        : (length * 2);
    constexpr unsigned int numGrids = 1 + int(CBT);
    unsigned int allBarLength = (numGrids * gridBarLength);
    if (numGrids > 1) allBarLength += (numGrids - 1) * GRID_SEP.length();
    return printMessageBar(msg, allBarLength + 1, fillChar);
}

#undef STATW_I
#undef STATW_D

} // End of Sudoku namespace.




static std::string createHSepString(unsigned int order) {
    std::string vbar = ' ' + std::string((((order * (order + 1)) + 1) * 2 - 1), '-');
    for (unsigned int i = 0; i <= order; i++) {
        // Insert crosses at vbar intersections.
        vbar[(2 * (order + 1) * i) + 1] = '+';
    }
    return std::move(vbar);
}
