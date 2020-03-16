#include "./trials.hpp"

#include "../util/ansi.hpp"

#include <iostream>
#include <iomanip>

namespace Sudoku::Trials {

template <Order O, bool CBT>
void ThreadFunc<O,CBT>::operator()(Solver* solver, const unsigned threadNum) {
    mutex.lock();
    if (threadNum != 0) {
        Solver *const oldSolver = solver;
        solver = new Solver(oldSolver->os);
        solver->setGenPath(oldSolver->getGenPath());
    }
    while (true) {
        // Attempt to generate a single solution:
        mutex.unlock();
            // CRITICAL SECTION:
            // This function call is the only section unguarded by the mutex.
            // That's fine. This covers the overwhelming majority of the work,
            // and everything else requires mutual exclusion to access shared
            // state and print outputs.
            SolverExitStatus exitStatus;
            const opcount_t numOperations = solver->generateSolution(exitStatus);
        mutex.lock();

        // Check break conditions:
        // Note that doing this _after_ attempting a trial (instead of before)
        // results in a tiny bit of wasted effort for the last [numThreads] or
        // so trials. I'm doing this because it makes the numOperations printing
        // nicer (otherwise there will be occasional visual gaps).
        if (trialsStopMethod == StopBy::TRIALS
            && totalTrials == trialsStopThreshold) {
            break;
        } else if (trialsStopMethod == StopBy::SUCCESSES
            && totalSuccesses >= trialsStopThreshold) {
            break;
        }

        // Print a progress indicator to stdout:
        if (totalTrials % COLS == 0) {
            trials_t trialsStopCurVal;
            switch (trialsStopMethod) {
            case StopBy::TRIALS:    trialsStopCurVal = totalTrials; break;
            case StopBy::SUCCESSES: trialsStopCurVal = totalSuccesses; break;
            default: trialsStopCurVal = 0; throw "unhandled enum case";
            }
            const unsigned pctDone = 100.0 * trialsStopCurVal / trialsStopThreshold;
            std::cout << "\n| " << std::setw(2) << pctDone << "% |";
        }
        totalTrials++;

        // Print the number of operations taken:
        if (exitStatus == SolverExitStatus::SUCCESS) {
            totalSuccesses++;
            solver->os << std::setw(solver->STATS_WIDTH) << numOperations;
        } else {
            if (solver->isPretty) {
                solver->os << Ansi::DIM.ON << std::setw(solver->STATS_WIDTH) << numOperations << Ansi::DIM.OFF;
            } else {
                solver->os << std::setw(solver->STATS_WIDTH) << "---";
            }
        } //solver->os << '~' << threadNum;
        if constexpr (solver->order > 4) solver->os << std::flush;

        // Save some stats for later diagnostics-printing:
        const opcount_t giveupCondVar
            = (GUM == GUM::E::OPERATIONS) ? numOperations
            : (GUM == GUM::E::BACKTRACKS) ? solver->getMaxBacktrackCount()
            : [](){ throw "unhandled GUM case"; return ~0; }();
        const unsigned binNum = NUM_BINS * (giveupCondVar) / solver->GIVEUP_THRESHOLD;
        binHitCount[binNum]++;
        binOpsTotal[binNum] += numOperations;
    }
    mutex.unlock();
    if (threadNum != 0) delete solver;
}

} // End of Sudoku::Trials namespace
