#include "./trials.hpp"

#include "../util/ansi.hpp"

#include <iostream>
#include <iomanip>

namespace Sudoku::Trials {

const std::string THROUGHPUT_COMMENTARY =
    "\n* Throughput here is in \"average successes per operation\". Tightening the"
    "\n  threshold induces more frequent giveups, but also reduces the operational"
    "\n  cost that giveups incur. Operations are proportional to time, and machine"
    "\n  independent. The visualization bars are purposely stretched to draw focus"
    "\n  to the optimal bin. Exercise prudence against stats from small datasets!";


template <Order O>
void ThreadFunc<O>::operator()(solver_t* solver, const unsigned threadNum) {
    if (threadNum != 0) {
        // TODO [bug] oldSolver can technically finish before we get here.
        solver_t *const oldSolver = solver;
        solver = new solver_t(oldSolver->os);
        solver->copySettingsFrom(*oldSolver);
    }
    mutex.lock();
    while (true) {
        // Attempt to generate a single solution:
        mutex.unlock();
            // CRITICAL SECTION:
            // This is the only section unguarded by the mutex. That's fine
            // because it covers the overwhelming majority of the work, and
            // everything else actually accesses shared state and resources.
            solver->generateSolution();
        mutex.lock();

        // Check break conditions:
        // Note that doing this _after_ attempting a trial (instead of before)
        // results in a tiny bit of wasted effort for the last [numThreads] or
        // so trials. I'm doing this to prevent some visual printing gaps).
        if (trialsStopCurVal() >= trialsStopThreshold) {
            break;
        }

        // Print a progress indicator to std::cout:
        {
        using Solver::ExitStatus;
        ExitStatus exitStatus = solver->prevGen.getExitStatus();
        const bool doOutputLine =
            (outputLvl == OutputLvl::EMIT_ALL) ||
            (outputLvl == OutputLvl::SUPPRESS_GIVEUPS && exitStatus == ExitStatus::SUCCESS);
        if (totalTrials % COLS == 0) {
            const unsigned newPercentDone = 100u * trialsStopCurVal() / trialsStopThreshold;
            if (doOutputLine) {
                std::cout << "\n| " << std::setw(2) << newPercentDone << "% |";
            } else if (outputLvl == OutputLvl::SILENT) {
                const int charDiff =
                    (newPercentDone * TABLE_SEPARATOR.size() / 100u)
                     - (percentDone * TABLE_SEPARATOR.size() / 100u);
                for (int i = 0; i < charDiff; i++) {
                    std::cout << Ansi::GREYSCALE_BLOCK_CHARS.back() << std::flush;
                }
            }
            percentDone = newPercentDone;
        }
        totalTrials++;
        if (exitStatus == ExitStatus::SUCCESS) {
            totalSuccesses++;
        }
        // Print the generated solution:
        if (doOutputLine) {
            // solver->os << '(' << threadNum << ')';
            solver->os << (solver->isPretty ? ' ' : '\n');
            solver->printSimple();
            if constexpr (solver->order > 4) solver->os << std::flush;
        }
        }

        // Save some stats for later diagnostics-printing:
        const Solver::opcount_t opCount = solver->prevGen.getOpCount();
        const Solver::opcount_t giveupCondVar
            = (Solver::gum == Solver::GUM::E::OPERATIONS) ? opCount
            : (Solver::gum == Solver::GUM::E::BACKTRACKS) ? solver->getMaxBacktrackCount()
            : [](){ throw "unhandled GUM case"; return ~0; }();
        const unsigned binNum = NUM_BINS * (giveupCondVar) / solver->GIVEUP_THRESHOLD;
        binHitCount[binNum]++;
        binOpsTotal[binNum] += opCount;
    }
    mutex.unlock();
    if (threadNum != 0) delete solver;
}

} // End of Sudoku::Trials namespace
