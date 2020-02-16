#include "grid.hpp"

#include <iostream>     // cout,
#include <iomanip>      // setbase, setw,
#include <fstream>      // fistream,
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
    totalGenCount = 0;
    successfulGenCount = 0;

    // Output formatting:
    if (isPretty) {
        benchedLocale = os.imbue(std::locale(os.getloc(), new MyNumpunct()));
    }
    os.precision(3);
    os << std::fixed;

    // Print help and start the REPL (read-execute-print-loop):
    std::cout << HELP_MESSAGE << std::endl;
    std::string command;
    do {
        std::cout << REPL_PROMPT;
        std::getline(std::cin, command);
    } while (runCommand(command));
}


template <Order O, bool CBT>
void Solver<O,CBT>::print(void) const {
    typename std::array<unsigned,area>::const_iterator idxMaxBacktracks;
    (void)idxMaxBacktracks;
    if constexpr (CBT) {
        idxMaxBacktracks = std::max_element(backtrackCounts.begin(), backtrackCounts.end());
        const area_t index = idxMaxBacktracks - backtrackCounts.begin();
        os << "max backtracks: " STATW_I << *idxMaxBacktracks
            // Print zero-indexed x,y coordinates:
            << " at (" << static_cast<unsigned>(getCol(index))
            << ',' << static_cast<unsigned>(getRow(index)) << ')' << '\n';
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
        for (length_t col = 0; col < length; col++) {
            if (isPretty && (col % order) == 0) os << " |"; // blkcol separator.
            os << ' ' << grid[row * length + col];
        }
        if constexpr (CBT) {
            if (isPretty) os << " |";
            os << GRID_SEP;
            for (length_t col = 0; col < length; col++) {
                if (isPretty && (col % order) == 0) os << " |"; // blkcol separator.
                printBacktrackStat(row * length + col, *idxMaxBacktracks);
            }
        }
            // if (isPretty) os << " |";
            // os << GRID_SEP;
            // for (length_t col = 0; col < length; col++) {
            //     if (isPretty && (col % order) == 0) os << " |"; // blkcol separator.
            //     os << ' ' << grid[row * length + col].biasIndex;
            // }
            // if (isPretty) os << " |";
            // os << GRID_SEP;
            // for (length_t col = 0; col < length; col++) {
            //     if (isPretty && (col % order) == 0) os << " |"; // blkcol separator.
            //     os << ' ' << rowBiases[row][col];
            // }
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
                // No solution could be found. Treat as if abort.
                totalGenCount++;
                return 0;
            }
            if constexpr (CBT) backtrackCounts[gridIndex]++;
            tvsIndex--;
        } else {
            tvsIndex++;
        }
        // Check if the giveup threshold has been exceeded:
        numOperations++;
        if (__builtin_expect(numOperations > GIVEUP_THRESHOLD, false)) {
            totalGenCount++;
            return 0;
        }
    }
    totalGenCount++;
    successfulGenCount++;
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
typename Solver<O,CBT>::length_t Solver<O,CBT>::tileNumNonCandidates(const area_t index) const noexcept {
    return __builtin_popcount(
          rowSymbolOccMasks[getRow(index)]
        | colSymbolOccMasks[getCol(index)]
        | blkSymbolOccMasks[getBlk(index)]
    );
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
bool Solver<O,CBT>::runCommand(std::string const& cmdLine) {
    // Purposely use cout instead of this.os.
    size_t tokenPos;
    const std::string cmdName = cmdLine.substr(0, tokenPos = cmdLine.find(" "));
    const std::string cmdArgs = cmdLine.substr(tokenPos + 1, std::string::npos);
    const auto it = COMMAND_MAP.find(cmdName);
    if (it == COMMAND_MAP.end()) {
        // No command name was matched
        std::cout << "command not found. enter \"help\" for the help menu." << std::endl;
        return true;
    }
    switch (it->second) {
        case CMD_HELP:
            std::cout << HELP_MESSAGE << std::endl;
            break;
        case CMD_QUIT:
            return false;
        case CMD_SOLVE: {
            std::ifstream puzzleFile(cmdArgs);
            if (!puzzleFile.good()) {
                std::cout << "the specified file could not be opened for reading." << std::endl;
            } else {
                solvePuzzlesFromFile(puzzleFile);
            }
            break; }
        case CMD_RUN_SINGLE:
            runNew();
            break;
        case CMD_RUN_MULTIPLE:
            try {
                runMultiple(std::stoul(cmdArgs));
            } catch (std::invalid_argument const& ia) {
                std::cout << "could not convert " << cmdArgs << " to an integer." << std::endl;
            }
            break;
        case CMD_SET_GENPATH:
            setGenPath(static_cast<GenPath>((genPath + 1) % (GenPath_MAX + 1)));
            std::cout << "generator path is now set to: " << GenPath_Names[genPath] << std::endl;
            break;
        default:
            break; // unreachable.
    }
    return true;
}


template <Order O, bool CBT>
void Solver<O,CBT>::solvePuzzlesFromFile(std::ifstream& puzzlesFile) {
    std::cout << "this has not yet been implemented yet" << std::endl;

    for (;;) {
        clear();
        // read a puzzle from the file
        generateSolution<true>();
        // write the solution to an output file.
        break; // TODO: remove me
    }
}


template <Order O, bool CBT>
void Solver<O,CBT>::runNew(void) {
    printMessageBar("START " + std::to_string(totalGenCount));
    clear();

    // Generate a new solution:
    const clock_t    clockStart = std::clock();
    const opcount_t numSolveOps = generateSolution<false>();
    const clock_t   clockFinish = std::clock();
    const double  processorTime = ((double)(clockFinish - clockStart)) / CLOCKS_PER_SEC;

    os << "num operations: " STATW_I << numSolveOps << '\n';
    os << "processor time: " STATW_D << processorTime << " seconds" << '\n';
    if (!isPretty) printMessageBar("", '-');
    print();
    printMessageBar((numSolveOps == 0) ? "ABORT" : "DONE");
}


template <Order O, bool CBT>
void Solver<O,CBT>::printBacktrackStat(const area_t index, unsigned const& worstCount) const {
    static const std::array<std::string, 4> GREYSCALE_BLOCK_CHARS = {
        // NOTE: Make sure that the initializer list size matches that
        // of the corresponding template argument. Compilers won't warn.
        // See https://cppreference.com/w/cpp/language/sizeof...#Example
        // for an example utility function I can make to avoid this problem.
        u8"\u2591", u8"\u2592", u8"\u2593", u8"\u2588",
    };

    if constexpr (CBT) {
        const unsigned int relativeIntensity
            = (double)(backtrackCounts[index] - 1)
            * GREYSCALE_BLOCK_CHARS.size()
            / worstCount
            ;
        auto const& intensityChar
            = (backtrackCounts[index] != 0)
            ? GREYSCALE_BLOCK_CHARS[relativeIntensity]
            : " ";
        os << intensityChar << intensityChar;
    } else {
        // This complains when the function is in a call path when
        // backtrack-counting is off.
        static_assert(CBT, "not avaliable when backtrack-counting is off.");
    }
}


template <Order O, bool CBT>
void Solver<O,CBT>::runMultiple(const trials_t trialsToRun) {
    static constexpr unsigned DEFAULT_STATS_COLS = ((unsigned[]){0,64,32,24,16,4,1})[order];
    static constexpr unsigned LINES_PER_FLUSH    = ((unsigned[]){0, 0, 0, 0, 6,1,1})[order];
    const unsigned PRINT_COLS = (GET_TERMINAL_COLUMNS(DEFAULT_STATS_COLS) - (isPretty ? 7 : 0)) / statsWidth;
    const unsigned BAR_WIDTH  = statsWidth * PRINT_COLS + (isPretty ? 7 : 0);

    trials_t giveups = 0;
    clock_t clockStart = std::clock();
    std::array<trials_t, TRIALS_NUM_BINS> successfulTrialBins    = {0,};
    std::array<double,   TRIALS_NUM_BINS> successfulSolveOpsBins = {0,};

    printMessageBar("START x" + std::to_string(trialsToRun), BAR_WIDTH);

    for (trials_t numTotalTrials = 0; numTotalTrials < trialsToRun;) {
        if (numTotalTrials % PRINT_COLS == 0) {
            const unsigned pctDone = 100.0 * numTotalTrials / trialsToRun;
            std::cout << "| " << std::setw(2) << pctDone << "% |";
        }
        clear();
        const opcount_t numSolveOps = generateSolution<false>();
        numTotalTrials++;
        if (numSolveOps == 0) {
            giveups++;
            os STATW_I << "---";
        } else {
            const unsigned int bin = (numSolveOps - 1) * TRIALS_NUM_BINS / GIVEUP_THRESHOLD;
            successfulTrialBins[bin]++;
            successfulSolveOpsBins[bin] += numSolveOps;
            os STATW_I << numSolveOps;
        }
        if (numTotalTrials % PRINT_COLS == 0) {
            if (isPretty && LINES_PER_FLUSH && (numTotalTrials % (LINES_PER_FLUSH * PRINT_COLS) == 0)) {
                // Runs are slower. Flush buffer more frequently.
                os << std::endl;
            } else {
                os << '\n';
            }
        }
    }
    if (trialsToRun % PRINT_COLS != 0) { os << '\n'; } // Last newline.

    // Print stats:
    const double processorSeconds = ((double)(std::clock() - clockStart) / CLOCKS_PER_SEC);
    printMessageBar("", BAR_WIDTH, '-');
    os << "trials aborted: " STATW_I << giveups << '\n';
    os << "processor time: " STATW_D << processorSeconds << " seconds (including I/O)" << '\n';
    if (processorSeconds > 10.0) {
        // Emit a beep sound if the trials took longer than ten processor seconds:
        std::cout << '\a' << std::flush;
    }

    // Print bins (work distribution):
    printMessageBar("", BAR_WIDTH, '-');
    printTrialsWorkDistribution(trialsToRun, successfulTrialBins, successfulSolveOpsBins);
    printMessageBar("DONE x" + std::to_string(trialsToRun), BAR_WIDTH);
    os << std::flush;
}


template <Order O, bool CBT>
void Solver<O,CBT>::printTrialsWorkDistribution(
    const trials_t trialsToRun,
    std::array<trials_t, TRIALS_NUM_BINS> const& successfulTrialBins,
    std::array<double,   TRIALS_NUM_BINS> const& successfulSolveOpsBins
) {
    os << "|  bin top  |   hits   |  throughput  |\n";
    os << "+-----------+----------+--------------+";
    opcount_t successfulTrialsAccum = 0;
    double  successfulSolveOpsAccum = 0.0;
    constexpr double BIN_WIDTH = (double)GIVEUP_THRESHOLD / successfulTrialBins.size();
    for (unsigned int i = 0; i < successfulTrialBins.size(); i++) {
        successfulTrialsAccum   += successfulTrialBins[i];
        successfulSolveOpsAccum += successfulSolveOpsBins[i];
        const double binCeiling = ((double)(i + 1) * BIN_WIDTH);
        const double throughput = successfulTrialsAccum / (successfulSolveOpsAccum
            + ((trialsToRun - successfulTrialsAccum) * binCeiling));
        if constexpr (order < 4) {
            os << "\n|" << std::setw(9) << (int)binCeiling;
        } else {
            os << "\n|" << std::setw(8) << (int)(binCeiling / 1000) << 'K';
        }
        os << "  |" << std::setw(8)  << successfulTrialBins[i];
        os << "  |" << std::setw(12) << std::scientific << throughput << std::fixed;
        os << "  |";
    }
    os << " <- current threshold\n";
    os << "+-----------+----------+--------------+\n";
    os << " * Throughput here is \"average successes per operation\". Tightening the"
        "\n   threshold induces more giveups, but also reduces the operational cost"
        "\n   giveups incur. Mathematically speaking, operations are proportional"
        "\n   to time, except operations are machine independent unlike time.\n";
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

} // End of Sudoku namespace.




static std::string createHSepString(unsigned int order) {
    std::string vbar = ' ' + std::string((((order * (order + 1)) + 1) * 2 - 1), '-');
    for (unsigned int i = 0; i <= order; i++) {
        // Insert crosses at vbar intersections.
        vbar[(2 * (order + 1) * i) + 1] = '+';
    }
    return std::move(vbar);
}
