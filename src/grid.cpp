#include "grid.hpp"

#include <iostream>     // cout,
#include <iomanip>      // setbase, setw,
#include <ctime>        // clock,
#include <numeric>      // iota,
#include <algorithm>    // random_shuffle,
#include <string>       // string,


#define STATW_I << std::setw(this->statsWidth)
#define STATW_D << std::setw(this->statsWidth + 4)
// ^mechanism to statically toggle alignment:


template <Sudoku::Order O>
Sudoku::Solver<O>::Solver(std::ostream& os):
    order       (O),
    length      (order  * order),
    area        (length * length),
    os          (os),
    isPretty    (&os == &std::cout),
    statsWidth  ((0.5 * length) + 3)
{
    grid.resize(area);
    rowBins.resize(length);
    colBins.resize(length);
    blkBins.resize(length);

    rowBiases.reserve(length);
    for (length_t i = 0; i < length; i++) {
        rowBiases.emplace_back(length + 1);
        std::iota(rowBiases[i].begin(), rowBiases[i].end(), 0);
    }
    traversalOrder.reserve(area);
    setGenPath(BLOCK_COLS);
    totalGenCount = 0;
    successfulGenCount = 0;

    // Output formatting:
    if (isPretty) {
        os.imbue(std::locale(os.getloc(), new MyNumpunct()));
    }
    os.precision(3);
    os << std::fixed;

    // Print help and start the REPL:
    std::cout << HELP_MESSAGE << std::endl;
    std::string command;
    do {
        std::cout << REPL_PROMPT;
        std::getline(std::cin, command);
    } while (runCommand(command));
}

template <Sudoku::Order O>
void Sudoku::Solver<O>::print(void) const {
    static std::string vbar = ' ' + std::string(((length + order + 1) * 2 - 1), '-');
    for (int i = 0; i <= order; i++) {
        vbar[1 + i * 2 * (order + 1)] = '+';
    }
    os << std::setbase(16);
    for (area_t i = 0; i < area; i++) {
        if ((i % length) == 0 && i != 0) {
            if (isPretty) os << " |";
            os << "\n";
        }
        if (isPretty && (i % (order * length) == 0)) {
            os << vbar << '\n';
        }
        if (isPretty && (i % order) == 0) os << " |";
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
        os << " |\n" << vbar;
    }
    os << std::setbase(10) << std::endl;
}

template <Sudoku::Order O>
void Sudoku::Solver<O>::clear(void) {
    // Initialize all values as empty:
    std::for_each(grid.begin(), grid.end(), [this](Tile& t){ t.clear(length); });
    std::fill(rowBins.begin(), rowBins.end(), 0);
    std::fill(colBins.begin(), colBins.end(), 0);
    std::fill(blkBins.begin(), blkBins.end(), 0);
}

template <Sudoku::Order O>
void Sudoku::Solver<O>::seed(const bool printInfo) {
    // Scramble each row's value-guessing-order:
    // note: must keep the <length>'th entry as <length>.
    for (auto& rowBias : rowBiases) {
        std::random_shuffle(rowBias.begin(), rowBias.end() - 1, myRandom);
    }
}


template <Sudoku::Order O>
typename Sudoku::Solver<O>::opcount_t Sudoku::Solver<O>::generateSolution(void) {
    static const opcount_t giveupThreshold = GIVEUP_THRESH_COEFF * (area * area * area);
    opcount_t numOperations = 0;
    register area_t index = 0;
    while (index < area) {
        // Push a new permutation:
        numOperations++;
        if (numOperations > giveupThreshold) {
            totalGenCount++;
            return 0;
        }
        if (isClear(setNextValid(traversalOrder[index]))) {
            // Pop and step backward:
            if (index == 0) {
                // No solution could be found. Treat as if abort.
                totalGenCount++;
                return 0;
            }
            index--;
        } else {
            index++;
        }
    }
    totalGenCount++;
    successfulGenCount++;
    return numOperations;
}

template <Sudoku::Order O>
typename Sudoku::Solver<O>::Tile const& Sudoku::Solver<O>::setNextValid(const area_t index) {
    occmask_t& rowBin = rowBins[getRow(index)];
    occmask_t& colBin = colBins[getCol(index)];
    occmask_t& blkBin = blkBins[getBlk(index)];
    //__builtin_ffsl(0); // TODO: investigate ways to use this for performance gains.

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
    t.biasIndex = biasIndex;
    return t;
}

template <Sudoku::Order O>
typename Sudoku::Solver<O>::length_t Sudoku::Solver<O>::tileNumNonCandidates(const area_t index) const noexcept {
    return __builtin_popcount(
          rowBins[getRow(index)]
        | colBins[getCol(index)]
        | blkBins[getBlk(index)]
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
        case RUN_SINGLE:
            runNew();
            break;
        case RUN_MULTIPLE:
            try {
                runMultiple(std::stoi(cmdArgs));
            } catch (std::invalid_argument const& ia) {
                std::cout << "could not convert " << cmdArgs << " to an integer." << std::endl;
            }
            break;
        case SET_GENPATH:
            setGenPath((GenPath)((genPath + 1) % (GenPath_MAX + 1)));
            std::cout << "generator path is now set to: " << Sudoku::GenPath_Names[genPath] << std::endl;
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

    os << "num operations: " STATW_I << numSolveOps << std::endl;
    os << "processor time: " STATW_D << processorTime << " sec" << std::endl;
    if (!isPretty) printMessageBar("", '-');
    print();
    printMessageBar((numSolveOps == 0) ? "ABORT" : "DONE");
}

template <Sudoku::Order O>
void Sudoku::Solver<O>::runMultiple(const unsigned int numAttempts) {
#define totalNumTrials      (totalGenCount      - initialTotalGenCount)
#define successfulNumTrials (successfulGenCount - initialSuccessfulGenCount)
    static const unsigned PRINT_COLS     = ((unsigned[]){0,0,16,12,8,1})[order];
    const auto initialTotalGenCount      = totalGenCount;
    const auto initialSuccessfulGenCount = successfulGenCount;
    double trialsNumSolveOps = 0.0;
    double totalClockTicks   = 0.0;
    printMessageBar("START x" + std::to_string(numAttempts), (statsWidth * PRINT_COLS));

    for (unsigned int attemptNum = 0; attemptNum < numAttempts; attemptNum++) {
        clear();
        seed(false);
        const clock_t    clockStart = std::clock();
        const opcount_t numSolveOps = generateSolution();
        totalClockTicks += (std::clock() - clockStart);
        trialsNumSolveOps += numSolveOps;
        if (numSolveOps == 0) {
            os STATW_I << "---";
        } else {
            os STATW_I << numSolveOps;
        }
        if (totalNumTrials % PRINT_COLS == 0) { os << '\n'; }
    }
    if (totalNumTrials % PRINT_COLS != 0) { os << '\n'; }

    // Print stats:
    const double averageNumSolveOps = (successfulNumTrials == 0) ? 0.0
        : ((double)trialsNumSolveOps / successfulNumTrials);
    printMessageBar("", (statsWidth * PRINT_COLS), '-');
    os << "num aborted trials: " STATW_I << (totalNumTrials - successfulNumTrials) << std::endl;
    os << "avg num operations: " STATW_D << averageNumSolveOps << std::endl;
    os << "sum processor time: " STATW_D << (totalClockTicks / CLOCKS_PER_SEC) << " sec" << std::endl;
    printMessageBar("DONE x" + std::to_string(numAttempts), (statsWidth * PRINT_COLS));

    // If printing to file and some time has passed, emit a beep sound:
    if (&os != &std::cout) {
        // TODO: count processor time please.
        std::cout << '\a' << std::flush;
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
void Sudoku::Solver<O>::printMessageBar(
    std::string const& msg,
    const char fillChar
) const {
    const unsigned int barLength = (isPretty)
        ? ((length + order + 1) * 2)
        : (length * 2);
    return printMessageBar(msg, barLength, fillChar);
}
