#include "repl.hpp"

#include <iostream>     // cout, endl
#include <iomanip>      // setw,


namespace Sudoku {

// Mechanism to statically toggle printing alignment:
// (#undef-ed before the end of this namespace)
#define STATW_I << std::setw(this->solver.statsWidth)
#define STATW_D << std::setw(this->solver.statsWidth + 4)


template <Order O, bool CBT>
Repl<O,CBT>::Repl(std::ostream& os):
    solver  (os),
    os      (os)
{
    // Print help menu and then start the REPL (read-execute-print-loop):
    std::cout << HELP_MESSAGE << std::endl;
    std::string command;
    do {
        std::cout << REPL_PROMPT;
        std::getline(std::cin, command);
    } while (runCommand(command));
};


template <Order O, bool CBT>
bool Repl<O,CBT>::runCommand(std::string const& cmdLine) {
    size_t tokenPos;
    // Very simple parsing: Assumes no leading spaces, and does not
    // trim leading or trailing spaces from the arguments substring.
    const std::string cmdName = cmdLine.substr(0, tokenPos = cmdLine.find(" "));
    const std::string cmdArgs = cmdLine.substr(tokenPos + 1, std::string::npos);
    const auto it = COMMAND_MAP.find(cmdName);
    if (it == COMMAND_MAP.end()) {
        // No command name was matched
        std::cout << "command \"" << cmdLine << "\" not found. enter \"help\" for the help menu." << std::endl;
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
            solver.setGenPath(static_cast<GenPath>((solver.getGenPath() + 1) % (GenPath_MAX + 1)));
            std::cout << "generator path is now set to: "
                << GenPath_Names[solver.getGenPath()] << std::endl;
            break;
        default:
            break; // unreachable.
    }
    return true;
}


template <Order O, bool CBT>
void Repl<O,CBT>::solvePuzzlesFromFile(std::ifstream& puzzlesFile) {
    // Put the string outside the loop
    for () {
        solver.clear();
        // Read a puzzle from the file:
        if (!solver.template loadPuzzleFromString<>(puzzleString)) {
            std::cout << "Could not use the input to read in a puzzle." << std::endl;
            // Just go try something else:
            continue;
        }
        solver.template generateSolution<true>();
        // TODO: write the solution to an output file / print it out.
        solver.print();
    }
}


template <Order O, bool CBT>
void Repl<O,CBT>::runNew(void) {
    solver.printMessageBar("START " + std::to_string(solver.totalGenCount));
    solver.clear();

    // Generate a new solution:
    const clock_t    clockStart = std::clock();
    const opcount_t numSolveOps = solver.template generateSolution<false>();
    const clock_t   clockFinish = std::clock();
    const double  processorTime = ((double)(clockFinish - clockStart)) / CLOCKS_PER_SEC;

    os << "num operations: " STATW_I << numSolveOps << '\n';
    os << "processor time: " STATW_D << processorTime << " seconds" << '\n';
    if (!solver.isPretty) solver.printMessageBar("", '-');
    solver.print();
    solver.printMessageBar((numSolveOps == 0) ? "ABORT" : "DONE");
}


template <Order O, bool CBT>
void Repl<O,CBT>::runMultiple(const trials_t trialsToRun) {
    constexpr unsigned DEFAULT_STATS_COLS = ((unsigned[]){0,64,32,24,16,4,1})[solver.order];
    constexpr unsigned LINES_PER_FLUSH    = ((unsigned[]){0, 0, 0, 0, 0,1,1})[solver.order];
    const unsigned PRINT_COLS = (GET_TERM_COLS(DEFAULT_STATS_COLS) - (solver.isPretty ? 7 : 0)) / solver.statsWidth;
    const unsigned BAR_WIDTH  = solver.statsWidth * PRINT_COLS + (solver.isPretty ? 7 : 0);

    trials_t giveups = 0;
    clock_t clockStart = std::clock();
    std::array<trials_t, TRIALS_NUM_BINS> successfulTrialBins    = {0,};
    std::array<double,   TRIALS_NUM_BINS> successfulSolveOpsBins = {0,};

    solver.printMessageBar("START x" + std::to_string(trialsToRun), BAR_WIDTH);

    for (trials_t numTotalTrials = 0; numTotalTrials < trialsToRun;) {
        if (numTotalTrials % PRINT_COLS == 0) {
            const unsigned pctDone = 100.0 * numTotalTrials / trialsToRun;
            std::cout << "| " << std::setw(2) << pctDone << "% |";
        }
        solver.clear();
        const opcount_t numSolveOps = solver.template generateSolution<false>();
        numTotalTrials++;
        if (numSolveOps == 0) {
            giveups++;
            os STATW_I << "---";
        } else {
            const unsigned int bin = (numSolveOps - 1) * TRIALS_NUM_BINS / Solver<O,CBT>::GIVEUP_THRESHOLD;
            successfulTrialBins[bin]++;
            successfulSolveOpsBins[bin] += numSolveOps;
            os STATW_I << numSolveOps;
        }
        if (numTotalTrials % PRINT_COLS == 0) {
            if constexpr (LINES_PER_FLUSH) {
                if (solver.isPretty && (numTotalTrials % (LINES_PER_FLUSH * PRINT_COLS) == 0)) {
                    // Runs are slower. Flush buffer more frequently.
                    os << std::endl;
                } else { os << '\n'; }
            }     else { os << '\n'; }
        }
    }
    if (trialsToRun % PRINT_COLS != 0) { os << '\n'; } // Last newline.

    // Print stats:
    const double processorSeconds = ((double)(std::clock() - clockStart) / CLOCKS_PER_SEC);
    solver.printMessageBar("", BAR_WIDTH, '-');
    os << "trials aborted: " STATW_I << giveups << '\n';
    os << "processor time: " STATW_D << processorSeconds << " seconds (including I/O)" << '\n';
    if (processorSeconds > 10.0) {
        // Emit a beep sound if the trials took longer than ten processor seconds:
        std::cout << '\a' << std::flush;
    }

    // Print bins (work distribution):
    solver.printMessageBar("", BAR_WIDTH, '-');
    printTrialsWorkDistribution(trialsToRun, successfulTrialBins, successfulSolveOpsBins);
    solver.printMessageBar("DONE x" + std::to_string(trialsToRun), BAR_WIDTH);
    os << std::flush;
}


template <Order O, bool CBT>
void Repl<O,CBT>::printTrialsWorkDistribution(
    const trials_t trialsToRun,
    std::array<trials_t, TRIALS_NUM_BINS> const& successfulTrialBins,
    std::array<double,   TRIALS_NUM_BINS> const& successfulSolveOpsBins
) {
    os << "|  bin top  |   hits   |  throughput  |\n";
    os << "+-----------+----------+--------------+";
    opcount_t successfulTrialsAccum = 0;
    double  successfulSolveOpsAccum = 0.0;
    constexpr double BIN_WIDTH = (double)Solver<O,CBT>::GIVEUP_THRESHOLD / successfulTrialBins.size();
    for (unsigned int i = 0; i < successfulTrialBins.size(); i++) {
        successfulTrialsAccum   += successfulTrialBins[i];
        successfulSolveOpsAccum += successfulSolveOpsBins[i];
        const double binCeiling = ((double)(i + 1) * BIN_WIDTH);
        const double throughput = successfulTrialsAccum / (successfulSolveOpsAccum
            + ((trialsToRun - successfulTrialsAccum) * binCeiling));
        if constexpr (solver.order < 4) {
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

#undef STATW_I
#undef STATW_D

} // End of Sudoku namespace.
