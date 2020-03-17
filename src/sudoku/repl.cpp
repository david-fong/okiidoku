#include "./repl.hpp"
#include "./trials.cpp"

#include <iostream>     // cout, endl,
#include <iomanip>      // setw,
#include <chrono>       // steady_clock::now, durationcast,
#include <math.h>       // pow,

#include <thread>
#include <mutex>


namespace Sudoku {

// Mechanism to statically toggle printing alignment:
// (#undef-ed before the end of this namespace)
#define STATW_I << std::setw(this->solver.STATS_WIDTH)
#define STATW_D << std::setw(this->solver.STATS_WIDTH + 4)


template <Order O>
Repl<O>::Repl(std::ostream& os):
    numExtraThreads([](){
        const unsigned HWC = std::thread::hardware_concurrency();
        // HWC is specified to be zero if unknown.
        return HWC ? std::min(MAX_EXTRA_THREADS, HWC) : 0;
    }()),
    solver  (os),
    os      (os)
{
    // Print diagnostics about Solver member size:
    std::cout << "\nsize of solver:  " << sizeof(solver) << " bytes";
    std::cout << "\ndefault genpath: " << solver.getGenPath() << std::endl;

    // Print help menu and then start the REPL (read-execute-print-loop):
    std::cout << HELP_MESSAGE << std::endl;
    std::string command;
    do {
        std::cout << REPL_PROMPT;
        std::getline(std::cin, command);
    } while (runCommand(command));
};


template <Order O>
bool Repl<O>::runCommand(std::string const& cmdLine) {
    size_t tokenPos;
    // Very simple parsing: Assumes no leading spaces, and does not
    // trim leading or trailing spaces from the arguments substring.
    const std::string cmdName = cmdLine.substr(0, tokenPos = cmdLine.find(" "));
    const std::string cmdArgs = (tokenPos == std::string::npos)
        ? "" : cmdLine.substr(tokenPos + 1, std::string::npos);
    const auto it = COMMAND_MAP.find(cmdName);
    if (it == COMMAND_MAP.end()) {
        // No command name was matched.
        std::cout << Ansi::RED.ON;
        std::cout << "command \"" << cmdLine << "\" not found."
            " enter \"help\" for the help menu.";
        std::cout << Ansi::RED.OFF << std::endl;
        return true;
    }
    switch (it->second) {
        case Command::HELP:
            std::cout << HELP_MESSAGE << '\n' << Ansi::DIM.ON
            << Solver::GenPath::OPTIONS_MENU << Ansi::DIM.OFF << std::endl;
            break;
        case Command::QUIT:
            return false;
        case Command::RUN_SINGLE:    runSingle();     break;
        case Command::CONTINUE_PREV: runSingle(true); break;
        case Command::RUN_TRIALS:    runMultiple(cmdArgs, Trials::StopBy::TRIALS);    break;
        case Command::RUN_SUCCESSES: runMultiple(cmdArgs, Trials::StopBy::SUCCESSES); break;
        case Command::SET_GENPATH:   solver.setGenPath(cmdArgs); break;
        case Command::SOLVE: {
            if (solver.loadPuzzleFromString(cmdArgs)) {
                // TODO: give better output if solver gives up. Maybe move to its own function.
                Solver::ExitStatus exitStatus;
                solver.template generateSolution<true>(exitStatus);
                solver.print();
                break;
            }
            std::ifstream puzzleFile(cmdArgs);
            if (!puzzleFile.good()) {
                std::cout << Ansi::RED.ON;
                std::cout << "the specified file could not be opened for reading.";
                std::cout << Ansi::RED.OFF << std::endl;
            } else {
                solvePuzzlesFromFile(puzzleFile);
            }
            break; }
    }
    return true;
}


template <Order O>
void Repl<O>::solvePuzzlesFromFile(std::ifstream& puzzlesFile) {
    // Put the string outside the loop since the space allocation
    // for proper input, should be all the same.
    std::string puzzleString;
    puzzleString.reserve(solver.area + 1);
    while (std::getline(puzzlesFile, puzzleString)) {
        // Read a puzzle from the file:
        if (!solver.loadPuzzleFromString(puzzleString)) {
            std::cout << Ansi::RED.ON;
            std::cout << "Could not use the input to read in a puzzle.";
            std::cout << Ansi::RED.OFF << std::endl;
            // Just go try something else:
            continue;
        }
        Solver::ExitStatus exitStatus;
        solver.template generateSolution<true>(exitStatus);

        // TODO [feat] Write the solution to an output file.
        solver.print();
    }
}


template <Order O>
void Repl<O>::runSingle(const bool contPrev) {
    solver.printMessageBar("START");

    // Generate a new solution:
    Solver::ExitStatus exitStatus;
    const clock_t    clockStart = std::clock();
    const opcount_t numSolveOps = solver.generateSolution(exitStatus, contPrev);
    const clock_t   clockFinish = std::clock();
    const double  processorTime = ((double)(clockFinish - clockStart)) / CLOCKS_PER_SEC;

    os << "\nprocessor time: " STATW_D << processorTime << " seconds";
    os << "\nnum operations: " STATW_I << numSolveOps;
    if constexpr (Solver::cbt) {
        os << "\nmax backtracks: " STATW_I << solver.getMaxBacktrackCount();
    }
    if (!solver.isPretty) solver.printMessageBar("", '-');
    solver.print();
    solver.printMessageBar((exitStatus == Solver::ExitStatus::SUCCESS) ? "DONE" : "ABORT");
    os << std::endl;
}


template <Order O>
void Repl<O>::runMultiple(
    const trials_t trialsStopThreshold,
    const Trials::StopBy trialsStopMethod
) {
    constexpr unsigned COLS_DFLT = ((unsigned[]){0,64,32,24,16,4,1})[O];
    const unsigned COLS = (solver.isPretty ? (GET_TERM_COLS(COLS_DFLT)-7) : COLS_DFLT) / solver.STATS_WIDTH;
    const unsigned BAR_WIDTH = solver.STATS_WIDTH * COLS + (solver.isPretty ? 7 : 0);

    // NOTE: The last bin is for trials that do not succeed.
    std::array<trials_t, Trials::NUM_BINS+1> binHitCount = {0,};
    std::array<double,   Trials::NUM_BINS+1> binOpsTotal = {0,};

    solver.printMessageBar("START x" + std::to_string(trialsStopThreshold), BAR_WIDTH);
    auto wallClockStart = std::chrono::steady_clock::now();
    auto procClockStart = std::clock();

    trials_t totalTrials = 0;
    {
        trials_t totalSuccesses = 0;
        std::mutex sharedStateMutex;
        Trials::SharedState sharedState {
            sharedStateMutex, COLS, trialsStopMethod, trialsStopThreshold,
            totalTrials, totalSuccesses, binHitCount, binOpsTotal,
        };

        // Start the threads:
        std::array<std::thread, MAX_EXTRA_THREADS> extraThreads;
        for (unsigned i = 0; i < numExtraThreads; i++) {
            auto threadFunc = Trials::ThreadFunc<O>(sharedState);
            extraThreads[i] = std::thread(std::move(threadFunc), &solver, i+1);
        } {
            auto thisThreadFunc = Trials::ThreadFunc<O>(sharedState);
            thisThreadFunc(&solver, 0);
        }
        for (unsigned i = 0; i < numExtraThreads; i++) {
            extraThreads[i].join();
        }
    }

    // Print stats:
    const double procSeconds = ((double)(std::clock() - procClockStart) / CLOCKS_PER_SEC);
    const double wallSeconds = ((double)[wallClockStart](){
        using namespace std::chrono;
        return duration_cast<microseconds>(steady_clock::now() - wallClockStart);
    }().count() / 1'000'000);
    const std::string secondsUnits = DIM_ON + " seconds (with I/O)" + DIM_OFF;
    solver.printMessageBar("", BAR_WIDTH, '-'); os
    << "\nhelper threads: " STATW_I << numExtraThreads
    << "\ngive-up method: " STATW_I << Solver::gum
    << "\ngenerator path: " STATW_I << solver.getGenPath()
    << "\nprocessor time: " STATW_D << procSeconds << secondsUnits
    << "\nreal-life time: " STATW_D << wallSeconds << secondsUnits
    ;
    if (wallSeconds > 10.0) {
        // Emit a beep sound if the trials took longer than ten processor seconds:
        std::cout << '\a' << std::flush;
    }
    // Print bins (work distribution):
    printTrialsWorkDistribution(totalTrials, binHitCount, binOpsTotal);
    solver.printMessageBar("DONE x" + std::to_string(trialsStopThreshold), BAR_WIDTH);
    os << std::endl;
}


template <Order O>
void Repl<O>::runMultiple(
    std::string const& trialsString,
    const Trials::StopBy stopByMethod
) {
    long stopByValue;
    try {
        stopByValue = std::stol(trialsString);
        if (stopByValue <= 0) {
            std::cout << Ansi::RED.ON;
            std::cout << "please provide a non-zero, positive integer.";
            std::cout << Ansi::RED.OFF << std::endl;
            return;
        }
    } catch (std::invalid_argument const& ia) {
        std::cout << Ansi::RED.ON;
        std::cout << "could not convert \"" << trialsString << "\" to an integer.";
        std::cout << Ansi::RED.OFF << std::endl;
        return;
    }
    runMultiple(static_cast<trials_t>(stopByValue), stopByMethod);
}


// TODO [qol] Add a column for operations
template <Order O>
void Repl<O>::printTrialsWorkDistribution(
    const trials_t totalTrials, // sum of entries of binHitCount
    std::array<trials_t, Trials::NUM_BINS+1> const& binHitCount,
    std::array<double,   Trials::NUM_BINS+1> const& binOpsTotal
) {
    const std::string THROUGHPUT_BAR_STRING = "--------------------------------";
    const std::string TABLE_SEPARATOR = "\n+-----------+----------+-----------+";
    const std::string TABLE_HEADER    = "\n|  bin bot  |   hits   |  speedup  |";

    // Calculate all throughputs before printing:
    // (done in its own loop so we can later print comparisons against the optimal bin)
    std::array<double, Trials::NUM_BINS+1> throughput;
    unsigned  bestThroughputBin     = 0u; {
    opcount_t successfulTrialsAccum = 0u;
    double  successfulSolveOpsAccum = 0.0;
    for (unsigned i = 0; i < Trials::NUM_BINS; i++) {
        successfulTrialsAccum   += binHitCount[i];
        successfulSolveOpsAccum += binOpsTotal[i];
        const double boundedGiveupOps = [&](){
            using namespace Sudoku::Solver; return
              (gum == GUM::E::OPERATIONS) ? ((double)(i+1) * solver.GIVEUP_THRESHOLD / Trials::NUM_BINS)
            // No nice way to do the below. If I want an exact thing, I would
            // need to change generateSolution to also track the numOperations
            // for some hypothetical, lower threshold, which would be for the
            // bottom of this bin. I would need to expose `Trials::NUM_BINS` to
            // the `Solver` class. As a temporary, pessimistic band-aid, I will
            // use the values for the next bin. Note that this will give `nan`
            // (0.0/0.0) if there is no data for the next bin.
            : (gum == GUM::E::BACKTRACKS) ? ((double)binOpsTotal[i+1] / binHitCount[i+1])
            : [](){ throw "unhandled GUM case"; return 0.0; }();
        }();
        const double boundedGiveupOpsTotal = (totalTrials - successfulTrialsAccum) * boundedGiveupOps;
        throughput[i] = (i == Trials::NUM_BINS)
            ? 0.0 // The last bin is for giveups. Throughput unknown.
            : successfulTrialsAccum / (successfulSolveOpsAccum + boundedGiveupOpsTotal);
        if (throughput[i] > throughput[bestThroughputBin]) {
            bestThroughputBin = i;
        }
    }}

    os << TABLE_SEPARATOR;
    os << TABLE_HEADER;
    os << TABLE_SEPARATOR;
    for (unsigned i = 0; i < binHitCount.size(); i++) {
        if (i == Trials::NUM_BINS) {
            // Print a special separator for the giveups row:
            os << TABLE_SEPARATOR;
        }
        // Bin Bottom column:
        const double binBottom  = (double)(i) * solver.GIVEUP_THRESHOLD / Trials::NUM_BINS;
        if constexpr (O < 4 || (O == 4 && Solver::gum == Solver::GUM::E::BACKTRACKS)) {
            os << "\n|" << std::setw(9) << (int)(binBottom);
        } else {
            os << "\n|" << std::setw(8) << (int)(binBottom / 1'000.0) << 'K';
        }
        // Bin Hit Count column:
        os << "  |";
        if (binHitCount[i] == 0) os << DIM_ON;
        os << std::setw(8) << binHitCount[i];
        if (binHitCount[i] == 0) os << DIM_OFF;

        // Speedup Column
        os << "  |" << std::setw(9);
        if (i == Trials::NUM_BINS) {
            os << "unknown";
        } else {
            //os << std::scientific << (throughput[i]) << std::fixed;
            os << 100.0 * (throughput[i] / throughput[Trials::NUM_BINS-1]);
        }
        // Closing right-edge:
        os << "  |";
        {
            // Print a bar to visualize throughput relative to tha
            // of the best. Note visual exaggeration via exponents
            // (the exponent value was chosen by taste / visual feel)
            const unsigned barLength = THROUGHPUT_BAR_STRING.length()
                * std::pow(throughput[i] / throughput[bestThroughputBin], 5);
            if (i != bestThroughputBin) os << DIM_ON;
            os << ' ' << THROUGHPUT_BAR_STRING.substr(0, barLength);
            if (i != bestThroughputBin) os << DIM_OFF;
        }
    }
    os << " <- current giveup threshold";
    os << TABLE_SEPARATOR;
    os << DIM_ON << Trials::THROUGHPUT_COMMENTARY << DIM_OFF;
}

#undef STATW_I
#undef STATW_D

} // End of Sudoku namespace.
