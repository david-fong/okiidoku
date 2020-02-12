#include "grid.hpp"

#include <iostream>     // cout,
#include <iomanip>      // setbase, setw,
#include <fstream>      // fistream,
#include <ctime>        // clock,
#include <numeric>      // iota,
#include <algorithm>    // random_shuffle,
#include <string>       // string,


// Mechanism to statically toggle printing alignment:
#define STATW_I << std::setw(this->statsWidth)
#define STATW_D << std::setw(this->statsWidth + 4)

static std::string createHSepString(unsigned int order);


template <Sudoku::Order O>
Sudoku::Solver<O>::Solver(std::ostream& os):
    os          (os),
    isPretty    (&os == &std::cout),
    gridHSepString(createHSepString(order))
{
    for (auto& rowBias : rowBiases) {
        std::iota(rowBias.begin(), rowBias.end(), 0);
    }
    setGenPath(DEFAULT_GENPATH);
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


template <Sudoku::Order O>
void Sudoku::Solver<O>::print(void) const {
    os << std::setbase(16);
    for (area_t i = 0; i < area; i++) {
        if ((i % length) == 0 && i != 0) {
            if (isPretty) os << " |";
            os << "\n"; // Line terminator.
        }
        if (isPretty && (i % (order * length) == 0)) {
            os << gridHSepString;
        }
        if (isPretty && (i % order) == 0) os << " |"; // blkcol separator.
        Tile const& t = grid.at(i);
        if (isClear(t)) {
            os << "  ";
        } else {
            os << " ";
            if (order < 5) {
                os << (uint16_t)t.value;
            } else {
                os << (char)('a' + t.value);
            }
        }
    }
    if (isPretty) {
        os << " |\n" << gridHSepString;
    }
    os << std::setbase(10) << std::endl;
}


template <Sudoku::Order O>
void Sudoku::Solver<O>::clear(void) {
    std::for_each(grid.begin(), grid.end(), [this](Tile& t){ t.clear(); });
    rowSymbolOccMasks.fill(0);
    colSymbolOccMasks.fill(0);
    blkSymbolOccMasks.fill(0);
    if (doCountBacktracks) backtrackCounts.fill(0);
}


template <Sudoku::Order O>
void Sudoku::Solver<O>::seed(const bool printInfo) {
    // Scramble each row's value-guessing-order:
    // note: must keep the <length>'th entry as <length>.
    for (auto& rowBias : rowBiases) {
        std::random_shuffle(rowBias.begin(), rowBias.end() - 1, MY_RANDOM);
    }
}




template <Sudoku::Order O>
typename Sudoku::opcount_t Sudoku::Solver<O>::generateSolution(void) {
    const bool doCountBacktracks = this->doCountBacktracks;
    opcount_t numOperations = 0;
    register area_t index = 0;
    while (index < area) {
        if (setNextValid(traversalOrder[index]) == TraversalDirection::BACK) {
            // Pop and step backward:
            if (__builtin_expect(index == 0, false)) {
                // No solution could be found. Treat as if abort.
                totalGenCount++;
                return 0;
            }
            if (doCountBacktracks) backtrackCounts[index]++;
            index--;
        } else {
            index++;
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


template <Sudoku::Order O>
typename Sudoku::TraversalDirection Sudoku::Solver<O>::setNextValid(const area_t index) {
    occmask_t& rowBin = rowSymbolOccMasks[getRow(index)];
    occmask_t& colBin = colSymbolOccMasks[getCol(index)];
    occmask_t& blkBin = blkSymbolOccMasks[getBlk(index)];

    Tile& t = grid[index];
    if (!isClear(t)) {
        // If the tile is currently already set, clear it:
        const occmask_t eraseMask = ~(0b1 << t.value);
        rowBin &= eraseMask;
        colBin &= eraseMask;
        blkBin &= eraseMask;
    }

    const occmask_t invalidBin = rowBin | colBin | blkBin;
    // Note: The below line is the only push to make the size of the
    // biasIndex field fit the range [0,order^2+1], but the trick is
    // that this will autowrap to zero, which takes the mod as desired.
    value_t biasIndex = (t.biasIndex + 1) % (length + 1);
    for (; biasIndex < length; biasIndex++) {
        const value_t value = rowBiases[getRow(index)][biasIndex];
        const occmask_t valBit = 0b1 << value;
        if (!(invalidBin & valBit)) {
            // If a valid value is found for this tile:
            rowBin |= valBit;
            colBin |= valBit;
            blkBin |= valBit;
            t.value = value;
            break;
        }
    }
    // This must go outside the search-loop for backtracks:
    t.biasIndex = biasIndex;
    return (isClear(t) ? BACK : FORWARD);
}


template <Sudoku::Order O>
typename Sudoku::Solver<O>::length_t Sudoku::Solver<O>::tileNumNonCandidates(const area_t index) const noexcept {
    return __builtin_popcount(
          rowSymbolOccMasks[getRow(index)]
        | colSymbolOccMasks[getCol(index)]
        | blkSymbolOccMasks[getBlk(index)]
    );
}


template <Sudoku::Order O>
void Sudoku::Solver<O>::setGenPath(const GenPath newGenPath) noexcept {
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






template <Sudoku::Order O>
bool Sudoku::Solver<O>::runCommand(std::string const& cmdLine) {
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
        case HELP:
            std::cout << HELP_MESSAGE << std::endl;
            break;
        case QUIT:
            return false;
        case SOLVE: {
            std::ifstream puzzleFile(cmdArgs);
            if (!puzzleFile.good()) {
                std::cout << "the specified file could not be opened for reading." << std::endl;
                break;
            }
            // TODO
            break; }
        case RUN_SINGLE:
            runNew();
            break;
        case RUN_MULTIPLE:
            try {
                runMultiple(std::stoul(cmdArgs));
            } catch (std::invalid_argument const& ia) {
                std::cout << "could not convert " << cmdArgs << " to an integer." << std::endl;
            }
            break;
        case SET_GENPATH:
            setGenPath(static_cast<GenPath>((genPath + 1) % (GenPath_MAX + 1)));
            std::cout << "generator path is now set to: " << Sudoku::GenPath_Names[genPath] << std::endl;
            break;
        case DO_BACKTRACK_COUNT:
            doCountBacktracks = !doCountBacktracks;
            std::cout << "backtracking activity monitoring is now: ";
            std::cout << ((doCountBacktracks) ? "on" : "off") << std::endl;
            break;
        default:
            break; // unreachable.
    }
    return true;
}


template <Sudoku::Order O>
void Sudoku::Solver<O>::runNew(void) {
    printMessageBar("START " + std::to_string(totalGenCount));
    clear();
    seed(true);

    // Generate a new solution:
    const clock_t    clockStart = std::clock();
    const opcount_t numSolveOps = generateSolution();
    const clock_t   clockFinish = std::clock();
    const double  processorTime = ((double)(clockFinish - clockStart)) / CLOCKS_PER_SEC;

    os << "num operations: " STATW_I << numSolveOps << '\n';
    os << "processor time: " STATW_D << processorTime << " seconds" << '\n';
    if (!isPretty) printMessageBar("", '-');
    print();
    printBacktrackStats();
    printMessageBar((numSolveOps == 0) ? "ABORT" : "DONE");
}


template <Sudoku::Order O>
void Sudoku::Solver<O>::printBacktrackStats(void) const {
    static const std::array<std::string, 5> GREYSCALE_BLOCK_CHARS = {
        " ", u8"\u2591", u8"\u2592", u8"\u2593", u8"\u2588",
    };
    if (!doCountBacktracks) {
        return;
    }
    auto sortedCounts = backtrackCounts;
    std::sort(sortedCounts.begin(), sortedCounts.end());

    // Print backtracking counters:
    printMessageBar("backtrack counters", '-');
    for (area_t i = 0; i < area; i++) {
        if ((i % length) == 0 && i != 0) {
            if (isPretty) os << " |";
            os << "\n"; // Line terminator.
        }
        if (isPretty && (i % (order * length) == 0)) {
            os << gridHSepString;
        }
        if (isPretty && (i % order) == 0) os << " |"; // blkcol separator.

        // TODO: find a way to make distribution more visible especially
        // for medium-valued counts. Maybe use sqrt or log function.
        // I know: use space char for zero, and start fading in at one.
        const double relativeIntensity = (double)backtrackCounts[i]
            * GREYSCALE_BLOCK_CHARS.size()
            / (sortedCounts[area - 1] + 1);
            // Note on above: `+ 1` to avoid edge case of getting array.size().
        auto const& intensityChar = GREYSCALE_BLOCK_CHARS.at((unsigned)relativeIntensity);
        os << intensityChar << intensityChar;
    }
    if (isPretty) {
        os << " |\n" << gridHSepString;
    }
    // Print a summary of the worst count values:
    printMessageBar("worst count values", '-');
    for (area_t i = area - length; i < area; i++) {
        os << std::setw(0.80 * statsWidth) << sortedCounts[i] << '\n';
    }
    os << '\n';
}


template <Sudoku::Order O>
void Sudoku::Solver<O>::runMultiple(const trials_t trialsToRun) {
    static constexpr unsigned DEFAULT_STATS_COLS = ((unsigned[]){0,64,32,24,16,4,1})[order];
    static constexpr unsigned LINES_PER_FLUSH    = ((unsigned[]){0, 0, 0, 0, 6,1,1})[order];
    const unsigned PRINT_COLS = (GET_TERMINAL_COLUMNS(DEFAULT_STATS_COLS) - (isPretty ? 7 : 0)) / statsWidth;
    const unsigned BAR_WIDTH  = statsWidth * PRINT_COLS + (isPretty ? 7 : 0);

    // Don't do backtracking during trials runs.
    const bool oldDoCountBacktracks = doCountBacktracks;
    doCountBacktracks = false;
    const bool isPretty = this->isPretty; // Will not change intermediately.

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
        seed(false);
        const opcount_t numSolveOps = generateSolution();
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

    // Restore old setting for counting backtracks:
    doCountBacktracks = oldDoCountBacktracks;
}

template<Sudoku::Order O>
void Sudoku::Solver<O>::printTrialsWorkDistribution(
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
        if (order < 4) {
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
    if (isPretty) { os \
        <<" * Throughput here is \"average successes per operation\". Tightening the"
        "\n   threshold induces more giveups, but also reduces the operational cost"
        "\n   giveups incur. Mathematically speaking, operations are proportional"
        "\n   to time, except operations are machine independent unlike time.\n";
    }
}




template <Sudoku::Order O>
void Sudoku::Solver<O>::printMessageBar(
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
    os << bar << std::endl;
}


template <Sudoku::Order O>
void Sudoku::Solver<O>::printMessageBar(std::string const& msg, const char fillChar) const {
    const unsigned int barLength = (isPretty)
        ? ((length + order + 1) * 2 + 1)
        : (length * 2);
    return printMessageBar(msg, barLength, fillChar);
}


static std::string createHSepString(unsigned int order) {
    std::string vbar = ' ' + std::string((((order * (order + 1)) + 1) * 2 - 1), '-') + '\n';
    for (unsigned int i = 0; i <= order; i++) {
        // Insert crosses at vbar intersections.
        vbar[(2 * (order + 1) * i) + 1] = '+';
    }
    return std::move(vbar);
}
