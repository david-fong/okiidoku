#include "./repl.hpp"

#include <iostream>     // cout, endl,
#include <iomanip>      // setw,
#include <chrono>       // steady_clock::now, durationcast,
#include <math.h>       // pow,


namespace Sudoku {

// Mechanism to statically toggle printing alignment:
// (#undef-ed before the end of this namespace)
#define STATW_I << std::setw(this->solver.STATS_WIDTH)
#define STATW_D << std::setw(this->solver.STATS_WIDTH + 4)
#if USE_ANSI_ESC
#define ANSI_DIM_ON  << "\e[2m"
#define ANSI_DIM_OFF << "\e[22m"
#else
#define ANSI_DIM_ON  << ""
#define ANSI_DIM_OFF << ""
#endif


template <Order O, bool CBT, GiveupMethod GUM>
Repl<O,CBT,GUM>::Repl(std::ostream& os):
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


template <Order O, bool CBT, GiveupMethod GUM>
bool Repl<O,CBT,GUM>::runCommand(std::string const& cmdLine) {
    size_t tokenPos;
    // Very simple parsing: Assumes no leading spaces, and does not
    // trim leading or trailing spaces from the arguments substring.
    const std::string cmdName = cmdLine.substr(0, tokenPos = cmdLine.find(" "));
    const std::string cmdArgs = (tokenPos == std::string::npos)
        ? "" : cmdLine.substr(tokenPos + 1, std::string::npos);
    const auto it = COMMAND_MAP.find(cmdName);
    if (it == COMMAND_MAP.end()) {
        // No command name was matched.
        std::cout << "command \"" << cmdLine << "\" not found. enter \"help\" for the help menu." << std::endl;
        return true;
    }
    switch (it->second) {
        case CMD_HELP:
            std::cout << HELP_MESSAGE << std::endl;
            break;
        case CMD_QUIT:
            return false;
        case CMD_RUN_SINGLE:    runSingle();     break;
        case CMD_CONTINUE_PREV: runSingle(true); break;
        case CMD_RUN_TRIALS:
        case CMD_RUN_SUCCESSES:
            try {
                const TrialsStopBy stopBy
                    = (it->second == CMD_RUN_TRIALS)    ? TOTAL_TRIALS
                    : (it->second == CMD_RUN_SUCCESSES) ? TOTAL_SUCCESSES
                    : [](){ throw "unhandled TSB case"; return (TrialsStopBy)~0; }();
                runMultiple(std::stoul(cmdArgs), stopBy);
            } catch (std::invalid_argument const& ia) {
                std::cout << "could not convert \"" << cmdArgs << "\" to an integer." << std::endl;
            }
            break;
        case CMD_SET_GENPATH:
            solver.setGenPath(cmdArgs);
            break;
        case CMD_SOLVE: {
            if (solver.loadPuzzleFromString(cmdArgs)) {
                // TODO: give better output if solver gives up. Maybe move to its own function.
                SolverExitStatus exitStatus;
                solver.template generateSolution<true>(exitStatus);
                solver.print();
                break;
            }
            std::ifstream puzzleFile(cmdArgs);
            if (!puzzleFile.good()) {
                std::cout << "the specified file could not be opened for reading." << std::endl;
            } else {
                solvePuzzlesFromFile(puzzleFile);
            }
            break; }
    }
    return true;
}


template <Order O, bool CBT, GiveupMethod GUM>
void Repl<O,CBT,GUM>::solvePuzzlesFromFile(std::ifstream& puzzlesFile) {
    // Put the string outside the loop since the space allocation
    // for proper input, should be all the same.
    std::string puzzleString;
    puzzleString.reserve(solver.area + 1);
    while (std::getline(puzzlesFile, puzzleString)) {
        // Read a puzzle from the file:
        if (!solver.loadPuzzleFromString(puzzleString)) {
            std::cout << "Could not use the input to read in a puzzle." << std::endl;
            // Just go try something else:
            continue;
        }
        SolverExitStatus exitStatus;
        solver.template generateSolution<true>(exitStatus);

        // TODO Write the solution to an output file.
        solver.print();
    }
}


template <Order O, bool CBT, GiveupMethod GUM>
void Repl<O,CBT,GUM>::runSingle(const bool contPrev) {
    solver.printMessageBar("START " + std::to_string(solver.getTotalGenCount()));

    // Generate a new solution:
    SolverExitStatus exitStatus;
    const clock_t    clockStart = std::clock();
    const opcount_t numSolveOps = solver.generateSolution(exitStatus, contPrev);
    const clock_t   clockFinish = std::clock();
    const double  processorTime = ((double)(clockFinish - clockStart)) / CLOCKS_PER_SEC;

    os << "processor time: " STATW_D << processorTime << " seconds" << '\n';
    os << "num operations: " STATW_I << numSolveOps << '\n';
    if constexpr (CBT) {
        os << "max backtracks: " STATW_I << solver.getMaxBacktrackCount() << '\n';
    }
    if (!solver.isPretty) solver.printMessageBar("", '-');
    solver.print();
    solver.printMessageBar((exitStatus == SolverExitStatus::SUCCESS) ? "DONE" : "ABORT");
}


template <Order O, bool CBT, GiveupMethod GUM>
void Repl<O,CBT,GUM>::runMultiple(const trials_t stopAfterValue, const TrialsStopBy stopAccordingTo) {
    constexpr unsigned DEFAULT_COLS     = ((unsigned[]){0,64,32,24,16,4,1})[solver.order];
    constexpr unsigned LINES_PER_FLUSH  = ((unsigned[]){0, 0, 0, 0, 0,1,1})[solver.order];
    const unsigned COLS = (solver.isPretty ? (GET_TERM_COLS(DEFAULT_COLS)-7) : DEFAULT_COLS) / solver.STATS_WIDTH;
    const unsigned BAR_WIDTH  = solver.STATS_WIDTH * COLS + (solver.isPretty ? 7 : 0);

    // NOTE: The last bin is for trials that do not succeed.
    std::array<trials_t, TRIALS_NUM_BINS+1> binHitCount = {0,};
    std::array<double,   TRIALS_NUM_BINS+1> binOpsTotal = {0,};

    solver.printMessageBar("START x" + std::to_string(stopAfterValue), BAR_WIDTH);
    auto wallClockStart = std::chrono::steady_clock::now();
    auto procClockStart = std::clock();

    trials_t numTotalTrials = 0;
    {
    trials_t doneTrialsCondVar = 0;
    trials_t numTotalSuccesses = 0;
    do {
        // Print a progress indicator to stdout:
        if (numTotalTrials % COLS == 0) {
            const unsigned pctDone = 100.0 * doneTrialsCondVar / stopAfterValue;
            std::cout << "| " << std::setw(2) << pctDone << "% |";
        }
        // Attempt to generate a single solution:
        SolverExitStatus exitStatus;
        const opcount_t numOperations = solver.generateSolution(exitStatus);
        numTotalTrials++;

        // Save some stats for later diagnostics-printing:
        const opcount_t giveupCondVar
            = (GUM == GiveupMethod::OPERATIONS) ? numOperations
            : (GUM == GiveupMethod::BACKTRACKS) ? solver.getMaxBacktrackCount()
            : [](){ throw "unhandled GUM case"; return ~0; }();
        const unsigned binNum = TRIALS_NUM_BINS * (giveupCondVar) / solver.GIVEUP_THRESHOLD;
        binHitCount[binNum]++;
        binOpsTotal[binNum] += numOperations;

        // Print the number of operations taken:
        if (exitStatus != SolverExitStatus::SUCCESS) {
            if (solver.isPretty) {
                os ANSI_DIM_ON;
                os STATW_I << numOperations;
                os ANSI_DIM_OFF;
            } else {
                os STATW_I << "---";
            }
        } else {
            numTotalSuccesses++;
            os STATW_I << numOperations;
        }
        // Newline-printing logic:
        if (numTotalTrials % COLS == 0) {
            if constexpr (LINES_PER_FLUSH) {
            if (solver.isPretty && (numTotalTrials % (LINES_PER_FLUSH * COLS) == 0)) {
                // Runs are slower. Flush buffer more frequently.
                os << std::endl;
            } else { os << '\n'; }
            } else { os << '\n'; }
        }
        switch (stopAccordingTo) {
            case TOTAL_TRIALS:    doneTrialsCondVar = numTotalTrials;
            case TOTAL_SUCCESSES: doneTrialsCondVar = numTotalSuccesses;
        }
    } while (doneTrialsCondVar < stopAfterValue);
    }
    if (stopAfterValue % COLS != 0) { os << '\n'; } // Last newline.

    // Print stats:
    const double procSeconds = ((double)(std::clock() - procClockStart) / CLOCKS_PER_SEC);
    const double wallSeconds = ((double)std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - wallClockStart).count() / 1'000'000);
    solver.printMessageBar("", BAR_WIDTH, '-');
    os << "give-up method: " STATW_I << GiveupMethod_Names[static_cast<unsigned>(GUM)] << '\n';
    os << "generator path: " STATW_I << solver.getGenPath() << '\n';
    os << "processor time: " STATW_D << procSeconds << " seconds (with I/O)" << '\n';
    os << "real-life time: " STATW_D << wallSeconds << " seconds (with I/O)" << '\n';
    if (wallSeconds > 10.0) {
        // Emit a beep sound if the trials took longer than ten processor seconds:
        std::cout << '\a' << std::flush;
    }

    // Print bins (work distribution):
    printTrialsWorkDistribution(numTotalTrials, binHitCount, binOpsTotal);
    solver.printMessageBar("DONE x" + std::to_string(stopAfterValue), BAR_WIDTH);
    os << std::flush;
}


template <Order O, bool CBT, GiveupMethod GUM>
void Repl<O,CBT,GUM>::printTrialsWorkDistribution(
    const trials_t numTotalTrials, // sum of entries of binHitCount
    std::array<trials_t, TRIALS_NUM_BINS+1> const& binHitCount,
    std::array<double,   TRIALS_NUM_BINS+1> const& binOpsTotal
) {
    const std::string THROUGHPUT_BAR_STRING = "--------------------------------";
    const std::string TABLE_SEPARATOR = "+-----------+----------+--------------+";

    // Calculate all throughputs before printing:
    // (done in its own loop so we can later print comparisons against the optimal bin)
    std::array<double, TRIALS_NUM_BINS+1> throughput;
    unsigned bestThroughputBin = 0; {
    opcount_t successfulTrialsAccum = 0;
    double  successfulSolveOpsAccum = 0.0;
    for (unsigned i = 0; i < TRIALS_NUM_BINS; i++) {
        successfulTrialsAccum   += binHitCount[i];
        successfulSolveOpsAccum += binOpsTotal[i];
        const double boundedGiveupOps
            = (GUM == GiveupMethod::OPERATIONS) ? ((double)(i+1) * solver.GIVEUP_THRESHOLD / TRIALS_NUM_BINS)
            // No nice way to do the below. If I want an exact thing, I would
            // need to change generateSolution to also track the numOperations
            // for some hypothetical, lower threshold, which would be for the
            // bottom of this bin. I would need to expose `TRIALS_NUM_BINS` to
            // the `Solver` class. As a temporary, pessimistic band-aid, I will
            // use the values for the next bin. Note that this will give `nan`
            // (0.0/0.0) if there is no data for the next bin.
            : (GUM == GiveupMethod::BACKTRACKS) ? ((double)binOpsTotal[i+1] / binHitCount[i+1])
            : [](){ throw "unhandled GUM case"; return 0.0; }();
        const double boundedGiveupOpsTotal = (numTotalTrials - successfulTrialsAccum) * boundedGiveupOps;
        throughput[i] = (i == TRIALS_NUM_BINS)
            ? 0.0 // The last bin is for giveups. Throughput unknown.
            : successfulTrialsAccum / (successfulSolveOpsAccum + boundedGiveupOpsTotal);
        if (throughput[i] > throughput[bestThroughputBin]) {
            bestThroughputBin = i;
        }
    }}

    os << TABLE_SEPARATOR;
    os << "\n|  bin bot  |   hits   |  throughput  |\n";
    os << TABLE_SEPARATOR;
    for (unsigned i = 0; i < binHitCount.size(); i++) {
        if (i == TRIALS_NUM_BINS) {
            // Print a special separator for the giveups row:
            os << '\n' << TABLE_SEPARATOR;
        }
        const double binBottom  = (double)(i) * solver.GIVEUP_THRESHOLD / TRIALS_NUM_BINS;
        if constexpr (solver.order < 4 || (solver.order == 4 && GUM == GiveupMethod::BACKTRACKS)) {
            os << "\n|" << std::setw(9) << (int)(binBottom);
        } else {
            os << "\n|" << std::setw(8) << (int)(binBottom / 1'000.0) << 'K';
        }
        os << "  |" << std::setw(8)  << binHitCount[i];
        os << "  |" << std::setw(12);
        if (i == TRIALS_NUM_BINS) { os << "unknown";
        } else { os << std::scientific << throughput[i] << std::fixed; }
        os << "  |";
        {
            // Print a bar to visualize throughput relative to tha
            // of the best. Note visual exaggeration via exponents
            // (the exponent value was chosen by taste / visual feel)
            const unsigned barLength = THROUGHPUT_BAR_STRING.length()
                * std::pow(throughput[i] / throughput[bestThroughputBin], 5);
            if (solver.isPretty && i != bestThroughputBin) os ANSI_DIM_ON;
            os << ' ' << THROUGHPUT_BAR_STRING.substr(0, barLength);
            if (solver.isPretty && i != bestThroughputBin) os ANSI_DIM_OFF;
        }
    }
    os << " <- current giveup threshold\n";
    os << TABLE_SEPARATOR;
    os ANSI_DIM_ON <<
        "\n * Throughput here is in \"average successes per operation\". Tightening the"
        "\n   threshold induces more frequent giveups, but also reduces the operational"
        "\n   cost that giveups incur. Operations are proportional to time, and machine"
        "\n   independent. The visualization bars are purposely stretched to draw focus"
        "\n   to the optimal bin.\n" ANSI_DIM_OFF;
}

#undef STATW_I
#undef STATW_D
#undef ANSI_DIM_ON
#undef ANSI_DIM_OFF

} // End of Sudoku namespace.
