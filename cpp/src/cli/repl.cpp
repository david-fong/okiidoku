#include "./repl.hpp"
#include "./trials.cpp"

#include <iostream>    // cout, endl,
#include <iomanip>     // setw,
#include <chrono>      // steady_clock::now, durationcast,
#include <cmath>      // pow,

#include <thread>
#include <mutex>


namespace solvent::cli {

// Mechanism to statically toggle printing alignment:
// (#undef-ed before the end of this namespace)
#define STATW_I << std::setw(this->solver.STATS_WIDTH)
#define STATW_D << std::setw(this->solver.STATS_WIDTH + 4)

const std::string TERMINAL_OUTPUT_TIPS =
"\nNote: You can run `tput rmam` in your shell to disable text wrapping."
"\nAlso, scrolling may be slower if the build flag `USE_ANSI_ESC is` on."
"\nIf UTF-8 characters are garbled on Windows, run `chcp.com 65001`.";


template <Order O>
Repl<O>::Repl(std::ostream& os):
	numExtraThreads([](){
		const unsigned HWC = std::thread::hardware_concurrency();
		// HWC is specified to be zero if unknown.
		return HWC ? std::min(MAX_EXTRA_THREADS, HWC) : 0;
	}()),
	gen (os),
	os     (os)
{
	setOutputLvl(OutputLvl::E::All);

	// Print diagnostics about Solver member size:
	std::cout
	<< "\nsolver obj size: " << sizeof(gen) << " bytes"
	<< "\ndefault genpath: " << gen.getGenPath();
	if constexpr (O > 3) {
		std::cout << '\n' << Ansi::DIM.ON << TERMINAL_OUTPUT_TIPS << Ansi::DIM.OFF;
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
		case E::Help:
			std::cout
			<< Command::HELP_MESSAGE << Ansi::DIM.ON
			<< '\n' <<       OutputLvl::OPTIONS_MENU
			<< '\n' << Solver::GenPath::OPTIONS_MENU
			<< Ansi::DIM.OFF << std::endl;
			break;
		case E::Quit:
			return false;
		case E::OutputLevel:setOutputLvl(cmdArgs); break;
		case E::SetGenPath:   gen.setGenPath(cmdArgs); break;
		case E::RunSingle:    runSingle();    break;
		case E::ContinuePrev: runSingle(true); break;
		case E::RunMultiple:    runMultiple(cmdArgs, trials::StopBy::TRIALS);   break;
		case E::RunMultipleOk: runMultiple(cmdArgs, trials::StopBy::SUCCESSES); break;
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
void Repl<O>::runSingle(const bool contPrev) {
	gen.printMessageBar("START");

	// Generate a new solution:
	const clock_t clockStart = std::clock();
	gen.generateSolution(contPrev);
	const double  processorTime = ((double)(std::clock() - clockStart)) / CLOCKS_PER_SEC;

	os << "\nprocessor time: " STATW_D << processorTime << " seconds";
	os << "\nnum operations: " STATW_I << gen.prev_gen.getOpCount();
	os << "\nmax backtracks: " STATW_I << gen.getMaxBacktrackCount();
	if (!gen.isPretty) gen.printMessageBar("", '-');
	gen.print();
	gen.printMessageBar((gen.prev_gen.getExitStatus() == Solver::ExitStatus::Ok) ? "DONE" : "ABORT");
	os << std::endl;
}


template <Order O>
void Repl<O>::runMultiple(
	const trials_t trialsStopThreshold,
	const trials::StopBy trialsStopMethod
) {
	const unsigned COLS = [this](){ // Never zero. Not used when writing to file.
		const unsigned termCols = GET_TERM_COLS();
		const unsigned cols = (termCols-7)/(gen.O4+1);
		return termCols ? (cols ? cols : 1) : ((unsigned[]){0,64,5,2,1,1,1})[O];
	}();
	const unsigned BAR_WIDTH = (gen.O4+1) * COLS + (gen.isPretty ? 7 : 0);
	// Note at above: the magic number `7` is the length of the progress indicator.

	// NOTE: The last bin is for trials that do not succeed.
	std::array<trials_t, trials::NUM_BINS+1> binHitCount = {0,};
	std::array<double,   trials::NUM_BINS+1> binOpsTotal = {0,};

	gen.printMessageBar("START x" + std::to_string(trialsStopThreshold), BAR_WIDTH);
	auto wallClockStart = std::chrono::steady_clock::now();
	auto procClockStart = std::clock();

	trials_t totalTrials = 0;
	{
		trials_t totalSuccesses = 0u;
		unsigned percentDone   = 0u;
		std::mutex sharedStateMutex;
		trials::SharedState sharedState {
			sharedStateMutex, COLS, getOutputLvl(),
			trialsStopMethod, trialsStopThreshold, percentDone,
			totalTrials, totalSuccesses, binHitCount, binOpsTotal,
		};
		if (getOutputLvl() == OutputLvl::E::Silent) {
			std::cout << '\n';
		}

		// Start the threads:
		std::array<std::thread, MAX_EXTRA_THREADS> extraThreads;
		for (unsigned i = 0; i < numExtraThreads; i++) {
			auto threadFunc = trials::ThreadFunc<O>(sharedState);
			extraThreads[i] = std::thread(std::move(threadFunc), &gen, i+1);
		} {
			auto thisThreadFunc = trials::ThreadFunc<O>(sharedState);
			thisThreadFunc(&gen, 0);
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
	gen.printMessageBar("", BAR_WIDTH, '-'); os
	<< "\nhelper threads: " STATW_I << numExtraThreads
	<< "\ngenerator path: " STATW_I << gen.getGenPath()
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
	gen.printMessageBar("DONE x" + std::to_string(trialsStopThreshold), BAR_WIDTH);
	os << std::endl;
}


template <Order O>
void Repl<O>::runMultiple(
	std::string const& trialsString,
	const trials::StopBy stopByMethod
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
	std::array<trials_t, trials::NUM_BINS+1> const& binHitCount,
	std::array<double,   trials::NUM_BINS+1> const& binOpsTotal
) {
	const std::string THROUGHPUT_BAR_STRING = "--------------------------------";

	// Calculate all throughputs before printing:
	// (done in its own loop so we can later print comparisons against the optimal bin)
	std::array<double, trials::NUM_BINS+1> throughput;
	std::array<double, trials::NUM_BINS+1> successfulTrialsAccumArr;
	unsigned  bestThroughputBin    = 0u; {
	opcount_t successfulTrialsAccum = 0u;
	double  successfulSolveOpsAccum = 0.0;
	for (unsigned i = 0; i < trials::NUM_BINS; i++) {
		successfulTrialsAccum   += binHitCount[i];
		successfulSolveOpsAccum += binOpsTotal[i];
		successfulTrialsAccumArr[i] = successfulTrialsAccum;
		const double boundedGiveupOps = ((double)binOpsTotal[i+1] / binHitCount[i+1]);
			// No nice way to do the above. If I want an exact thing, I would
			// need to change generateSolution to also track the numOperations
			// for some hypothetical, lower threshold, which would be for the
			// bottom of this bin. I would need to expose `trials::NUM_BINS` to
			// the `Solver` class. As a temporary, pessimistic band-aid, I will
			// use the values for the next bin. Note that this will give `nan`
			// (0.0/0.0) if there is no data for the next bin.
		const double boundedGiveupOpsTotal = (totalTrials - successfulTrialsAccum) * boundedGiveupOps;
		throughput[i] = successfulTrialsAccum / (successfulSolveOpsAccum + boundedGiveupOpsTotal);
		if (throughput[i] > throughput[bestThroughputBin]) {
			bestThroughputBin = i;
		}
	}}
	throughput[trials::NUM_BINS] = 0.0; // unknown.
	successfulTrialsAccumArr[trials::NUM_BINS] = 0.0;

	os << trials::TABLE_SEPARATOR;
	os << trials::TABLE_HEADER;
	os << trials::TABLE_SEPARATOR;
	for (unsigned i = 0; i < binHitCount.size(); i++) {
		if (i == trials::NUM_BINS) {
			// Print a special separator for the giveups row:
			os << trials::TABLE_SEPARATOR;
		}
		// Bin Bottom column:
		const double binBottom  = (double)(i) * gen.GIVEUP_THRESHOLD / trials::NUM_BINS;
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
		if (i == trials::NUM_BINS) {
			os << "unknown";
		} else {
			//os << std::scientific << (throughput[i]) << std::fixed;
			os << 100.0 * (throughput[i] / throughput[trials::NUM_BINS-1]);
		}
		// Closing right-edge:
		os << "  |";

		// Print a bar to visualize throughput relative to tha
		// of the best. Note visual exaggeration via exponents
		// (the exponent value was chosen by taste / visual feel)
		const unsigned barLength = THROUGHPUT_BAR_STRING.length()
			* std::pow(throughput[i] / throughput[bestThroughputBin], static_cast<int>(20.0/O));
		if (i != bestThroughputBin) os << DIM_ON;
		os << ' ' << THROUGHPUT_BAR_STRING.substr(0, barLength);
		if (i != bestThroughputBin) os << DIM_OFF;
	}
	os << " <- current giveup threshold";
	os << trials::TABLE_SEPARATOR;
	os << DIM_ON << trials::THROUGHPUT_COMMENTARY << DIM_OFF;
}

#undef STATW_I
#undef STATW_D

}
