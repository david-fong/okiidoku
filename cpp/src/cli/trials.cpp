#include "./trials.hpp"
#include "../util/ansi.hpp"

#include <iostream>
#include <iomanip>

namespace solvent::cli::trials {

const std::string THROUGHPUT_COMMENTARY =
	"\n* Throughput here is in \"average successes per operation\". Tightening the"
	"\n  threshold induces more frequent giveups, but also reduces the operational"
	"\n  cost that giveups incur. Operations are proportional to time, and machine"
	"\n  independent. The visualization bars are purposely stretched to draw focus"
	"\n  to the optimal bin. Exercise prudence against stats from small datasets!";


template <Order O>
void ThreadFunc<O>::operator()(generator_t* gen, const unsigned threadNum) {
	if (threadNum != 0) {
		// TODO [bug] oldSolver can technically finish before we get here.
		generator_t const*const oldSolver = gen;
		gen = new generator_t(oldSolver->os);
		gen->copy_settings_from(*oldSolver);
	}
	mutex.lock();
	while (true) {
		// Attempt to generate a single solution:
		mutex.unlock();
			// CRITICAL SECTION:
			// This is the only section unguarded by the mutex. That's fine
			// because it covers the overwhelming majority of the work, and
			// everything else actually accesses shared state and resources.
			gen->generateSolution();
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
			using lib::gen::ExitStatus;
			ExitStatus exitStatus = gen->prev_gen.getExitStatus();
			const bool doOutputLine =
				(outputLvl == OutputLvl::E::All) ||
				(outputLvl == OutputLvl::E::NoGiveups && exitStatus == ExitStatus::Ok);
			if (totalTrials % COLS == 0) {
				const unsigned newPercentDone = 100u * trialsStopCurVal() / trialsStopThreshold;
				if (doOutputLine) {
					std::cout << "\n| " << std::setw(2) << newPercentDone << "% |";
				} else if (outputLvl == OutputLvl::E::Silent) {
					const int charDiff =
						(newPercentDone * TABLE_SEPARATOR.size() / 100u)
						- (percentDone * TABLE_SEPARATOR.size() / 100u);
					for (int i = 0; i < charDiff; i++) {
						std::cout << Ansi::BLOCK_CHARS[2] << std::flush;
					}
				}
				percentDone = newPercentDone;
			}
			totalTrials++;
			if (exitStatus == ExitStatus::Ok) {
				totalSuccesses++;
			}
			// Print the generated solution:
			if (doOutputLine) {
				// gen->os << '(' << threadNum << ')';
				gen->os << (gen->isPretty ? ' ' : '\n');
				gen->printSimple();
				if constexpr (gen->O1 > 4) gen->os << std::flush;
			}
		}

		// Save some stats for later diagnostics-printing:
		const unsigned binNum = NUM_BINS * (gen->getMaxBacktrackCount()) / gen->GIVEUP_THRESHOLD;
		binHitCount[binNum]++;
		binOpsTotal[binNum] += gen->prev_gen.getOpCount();
	}
	mutex.unlock();
	if (threadNum != 0) delete gen;
}

}
