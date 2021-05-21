#ifndef HPP_SOLVENT_CLI_TRIALS
#define HPP_SOLVENT_CLI_TRIALS

#include "../lib/gen/mod.hpp"

#include <array>
#include <string>
#include <mutex>

namespace solvent::cli {

typedef unsigned long trials_t;

namespace trials {

	constexpr unsigned NUM_BINS = 20u;
	enum class StopBy : unsigned {
		TRIALS,
		SUCCESSES,
	};

	const std::string TABLE_SEPARATOR = "\n+-----------+----------+----------------+-----------+-----------+";
	const std::string TABLE_HEADER    = "\n|  bin bot  |   hits   |   operations   |  giveup%  |  speedup  |";

	struct SharedState {
		std::mutex&     mutex;
		const unsigned  COLS;
		const cli::OutputLvl::E outputLvl;
		const StopBy    trialsStopMethod;
		const trials_t  trialsStopThreshold;
		unsigned&       percentDone;
		trials_t&       totalTrials;
		trials_t&       totalSuccesses;
		std::array<trials_t, NUM_BINS+1>& binHitCount;
		std::array<double,   NUM_BINS+1>& binOpsTotal;
	};

	/**
	 * A helper for `Repl::runMultiple`.
	 *
	 * Note: Since it is only ever used there, the include guards are not
	 * absolutely necessary, but it doesn't hurt to add them anyway.
	 */
	template <Order O>
	class ThreadFunc final : private SharedState {
	public:
		using generator_t  = class lib::gen::Generator<O>;
		using OutputLvl = Repl::OutputLvl::E;
	public:
		ThreadFunc(void) = delete;
		explicit ThreadFunc(SharedState s) : SharedState(s) {};
		inline void operator()(generator_t* gen, unsigned threadNum);
	private:
		trials_t trialsStopCurVal(void) const {
			switch (trialsStopMethod) {
				case StopBy::TRIALS:    return totalTrials;
				case StopBy::SUCCESSES: return totalSuccesses;
				default: throw "unhandled enum case";
			}
		}
	};

}
}

#endif