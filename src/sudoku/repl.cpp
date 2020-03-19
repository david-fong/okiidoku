#include "./repl.hpp"
#include "./trials.cpp"

#include <iostream>     // cout, endl,
#include <iomanip>      // setw,
#include <chrono>       // steady_clock::now, durationcast,
#include <math.h>       // pow,

#include <thread>
#include <mutex>


namespace Sudoku::Repl {

// Mechanism to statically toggle printing alignment:
// (#undef-ed before the end of this namespace)
#define STATW_I << std::setw(this->solver.STATS_WIDTH)
#define STATW_D << std::setw(this->solver.STATS_WIDTH + 4)

const std::string TERMINAL_TEXT_FORMATTING_NOTES =
"\nNote: You can run `tput rmam` in your shell to disable text wrapping."
"\nAlso, scrolling may be slower if the build flag `USE_ANSI_ESC is` on.";


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
    setOutputLvl(OutputLvl::E::EMIT_ALL);

    // Print diagnostics about Solver member size:
    std::cout
    << "\nsolver obj size: " << sizeof(solver) << " bytes"
    << "\ndefault genpath: " << solver.getGenPath();
    if constexpr (O > 3) {
        std::cout << '\n' << Ansi::DIM.ON << TERMINAL_TEXT_FORMATTING_NOTES << Ansi::DIM.OFF;
    }
    std::cout << std::endl;

    // Print help menu and then start the REPL (read-execute-print-loop):
    std::cout << Command::HELP_MESSAGE << std::endl;
    std::string command;
    do {
        std::cout << PROMPT;
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
        ? "" :  cmdLine.substr(tokenPos + 1, std::string::npos);
    const auto it = Command::MAP.find(cmdName);
    if (it == Command::MAP.end()) {
        // No command name was matched.
        std::cout << Ansi::RED.ON;
        std::cout << "command \"" << cmdLine << "\" not found."
            " enter \"help\" for the help menu.";
        std::cout << Ansi::RED.OFF << std::endl;
        return true;
    }
    switch (it->second) {
        using Command::E;
        case E::HELP:
            std::cout
            << Command::HELP_MESSAGE << Ansi::DIM.ON
            << '\n' <<       OutputLvl::OPTIONS_MENU
            << '\n' << Solver::GenPath::OPTIONS_MENU
            << Ansi::DIM.OFF << std::endl;
            break;
        case E::QUIT:
            return false;
        case E::OUTPUT_LEVEL:setOutputLvl(cmdArgs); break;
        case E::SET_GENPATH:   solver.setGenPath(cmdArgs); break;
        case E::RUN_SINGLE:    runSingle();     break;
        case E::CONTINUE_PREV: runSingle(true); break;
        case E::RUN_TRIALS:    runMultiple(cmdArgs, Trials::StopBy::TRIALS);    break;
        case E::RUN_SUCCESSES: runMultiple(cmdArgs, Trials::StopBy::SUCCESSES); break;
        case E::SOLVE: {
            if (solver.loadPuzzleFromString(cmdArgs)) {
                // TODO: give better output if solver gives up. Maybe move to its own function.
                solver.template generateSolution<true>();
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
OutputLvl::E Repl<O>::setOutputLvl(OutputLvl::E newOutputLvl) {
    const OutputLvl::E oldOutputLvl = this->outputLvl;
    this->outputLvl = newOutputLvl;
    return oldOutputLvl;
}


template <Order O>
OutputLvl::E Repl<O>::setOutputLvl(std::string const& newOutputLvlString) {
    std::cout << "\noutput level is ";
    if (newOutputLvlString.empty()) {
        std::cout << "currently set to: " << getOutputLvl() << std::endl;
        return getOutputLvl();
    }
    for (unsigned i = 0; i < OutputLvl::size; i++) {
        if (newOutputLvlString.compare(OutputLvl::NAMES[i]) == 0) {
            if (OutputLvl::E{i} == getOutputLvl()) {
                std::cout << "already set to: ";
            } else {
                std::cout << "now set to: ";
                setOutputLvl(OutputLvl::E{i});
            }
            std::cout << getOutputLvl() << std::endl;
            return getOutputLvl();
        }
    }
    // unsuccessful return:
    std::cout << getOutputLvl() << " (unchanged).\n"
        << Ansi::RED.ON << '"' << newOutputLvlString
        << "\" is not a valid output level name.\n"
        << OutputLvl::OPTIONS_MENU << Ansi::RED.OFF << std::endl;
    return getOutputLvl();
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
        solver.template generateSolution<true>();

        // TODO [feat] Write the solution to an output file.
        solver.print();
    }
}


template <Order O>
void Repl<O>::runSingle(const bool contPrev) {
    solver.printMessageBar("START");

    // Generate a new solution:
    const clock_t clockStart = std::clock();
    solver.generateSolution(contPrev);
    const double  processorTime = ((double)(std::clock() - clockStart)) / CLOCKS_PER_SEC;

    os << "\nprocessor time: " STATW_D << processorTime << " seconds";
    os << "\nnum operations: " STATW_I << solver.prevGen.getOpCount();
    os << "\nmax backtracks: " STATW_I << solver.getMaxBacktrackCount();
    if (!solver.isPretty) solver.printMessageBar("", '-');
    solver.print();
    solver.printMessageBar((solver.prevGen.getExitStatus() == Solver::ExitStatus::SUCCESS) ? "DONE" : "ABORT");
    os << std::endl;
}


template <Order O>
void Repl<O>::runMultiple(
    const trials_t trialsStopThreshold,
    const Trials::StopBy trialsStopMethod
) {
    const unsigned COLS = [this](){ // Never zero. Not used when writing to file.
        const unsigned termCols = GET_TERM_COLS();
        const unsigned cols = (termCols-7)/(solver.area+1);
        return termCols ? (cols ? cols : 1) : ((unsigned[]){0,64,5,2,1,1,1})[O];
    }();
    const unsigned BAR_WIDTH = (solver.area+1) * COLS + (solver.isPretty ? 7 : 0);
    // Note at above: the magic number `7` is the length of the progress indicator.

    // NOTE: The last bin is for trials that do not succeed.
    std::array<trials_t, Trials::NUM_BINS+1> binHitCount = {0,};
    std::array<double,   Trials::NUM_BINS+1> binOpsTotal = {0,};

    solver.printMessageBar("START x" + std::to_string(trialsStopThreshold), BAR_WIDTH);
    auto wallClockStart = std::chrono::steady_clock::now();
    auto procClockStart = std::clock();

    trials_t totalTrials = 0;
    {
        trials_t totalSuccesses = 0u;
        unsigned percentDone    = 0u;
        std::mutex sharedStateMutex;
        Trials::SharedState sharedState {
            sharedStateMutex, COLS, getOutputLvl(),
            trialsStopMethod, trialsStopThreshold, percentDone,
            totalTrials, totalSuccesses, binHitCount, binOpsTotal,
        };
        if (getOutputLvl() == OutputLvl::E::SILENT) {
            std::cout << '\n';
        }

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
    << "\ngenerator path: " STATW_I << solver.getGenPath()
    // TODO [stats] For total successes and total trieals.
    << "\nprocessor time: " STATW_D << procSeconds << secondsUnits
    << "\nreal-life time: " STATW_D << wallSeconds << secondsUnits;
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


template <Order O>
void Repl<O>::printTrialsWorkDistribution(
    const trials_t totalTrials, // sum of entries of binHitCount
    std::array<trials_t, Trials::NUM_BINS+1> const& binHitCount,
    std::array<double,   Trials::NUM_BINS+1> const& binOpsTotal
) {
    const std::string THROUGHPUT_BAR_STRING = "--------------------------------";

    // Calculate all throughputs before printing:
    // (done in its own loop so we can later print comparisons against the optimal bin)
    std::array<double, Trials::NUM_BINS+1> throughput;
    std::array<double, Trials::NUM_BINS+1> successfulTrialsAccumArr;
    unsigned  bestThroughputBin     = 0u; {
    opcount_t successfulTrialsAccum = 0u;
    double  successfulSolveOpsAccum = 0.0;
    for (unsigned i = 0; i < Trials::NUM_BINS; i++) {
        successfulTrialsAccum   += binHitCount[i];
        successfulSolveOpsAccum += binOpsTotal[i];
        successfulTrialsAccumArr[i] = successfulTrialsAccum;
        const double boundedGiveupOps = ((double)binOpsTotal[i+1] / binHitCount[i+1]);
            // No nice way to do the above. If I want an exact thing, I would
            // need to change generateSolution to also track the numOperations
            // for some hypothetical, lower threshold, which would be for the
            // bottom of this bin. I would need to expose `Trials::NUM_BINS` to
            // the `Solver` class. As a temporary, pessimistic band-aid, I will
            // use the values for the next bin. Note that this will give `nan`
            // (0.0/0.0) if there is no data for the next bin.
        const double boundedGiveupOpsTotal = (totalTrials - successfulTrialsAccum) * boundedGiveupOps;
        throughput[i] = successfulTrialsAccum / (successfulSolveOpsAccum + boundedGiveupOpsTotal);
        if (throughput[i] > throughput[bestThroughputBin]) {
            bestThroughputBin = i;
        }
    }}
    throughput[Trials::NUM_BINS] = 0.0; // unknown.
    successfulTrialsAccumArr[Trials::NUM_BINS] = 0.0;

    os << Trials::TABLE_SEPARATOR;
    os << Trials::TABLE_HEADER;
    os << Trials::TABLE_SEPARATOR;
    for (unsigned i = 0; i < binHitCount.size(); i++) {
        if (i == Trials::NUM_BINS) {
            // Print a special separator for the giveups row:
            os << Trials::TABLE_SEPARATOR;
        }
        // Bin Bottom column:
        const double binBottom  = (double)(i) * solver.GIVEUP_THRESHOLD / Trials::NUM_BINS;
        if constexpr (O <= 4) {
            os << "\n|" << std::setw(9) << (int)(binBottom);
        } else {
            os << "\n|" << std::setw(8) << (int)(binBottom / 1'000.0) << 'K';
        }
        // Bin Hit Count column:
        os << "  |";
        if (binHitCount[i] == 0) os << DIM_ON;
        os << std::setw(8) << binHitCount[i];
        if (binHitCount[i] == 0) os << DIM_OFF;

        // Operation Count column:
        os << "  |";
        if (binHitCount[i] == 0) os << DIM_ON;
        os << std::setw(13) << unsigned(binOpsTotal[i] / ((O<5)?1:1000));
        os << ((O<5)?' ':'K');
        if (binOpsTotal[i] == 0) os << DIM_OFF;

        // Giveup Percentage column:
        os << "  |";
        os << std::setw(9) << (100.0 * (totalTrials - successfulTrialsAccumArr[i]) / totalTrials);

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

        // Print a bar to visualize throughput relative to tha
        // of the best. Note visual exaggeration via exponents
        // (the exponent value was chosen by taste / visual feel)
        const unsigned barLength = THROUGHPUT_BAR_STRING.length()
            * std::pow(throughput[i] / throughput[bestThroughputBin], 5);
        if (i != bestThroughputBin) os << DIM_ON;
        os << ' ' << THROUGHPUT_BAR_STRING.substr(0, barLength);
        if (i != bestThroughputBin) os << DIM_OFF;
    }
    os << " <- current giveup threshold";
    os << Trials::TABLE_SEPARATOR;
    os << DIM_ON << Trials::THROUGHPUT_COMMENTARY << DIM_OFF;
}

#undef STATW_I
#undef STATW_D

} // End of Sudoku namespace.
