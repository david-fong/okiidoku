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
		Trials,
		Successes,
	};

	const std::string TABLE_SEPARATOR = "\n+-----------+----------+----------------+-----------+-----------+";
	const std::string TABLE_HEADER    = "\n|  bin bot  |   hits   |   operations   |  giveup%  |  speedup  |";

	struct SharedState {
		std::mutex&     mutex;
		const unsigned  COLS;
		const cli::OutputLvl::E output_level;
		const StopBy    trials_stop_method;
		const trials_t  trials_stop_threshold;
		unsigned&       pct_done;
		trials_t&       total_trials;
		trials_t&       total_successes;
		std::array<trials_t, NUM_BINS+1>& bin_hit_count;
		std::array<double,   NUM_BINS+1>& bin_ops_total;
	};

	/**
	 * A helper for `Repl::run_multiple`.
	 *
	 * Note: Since it is only ever used there, the include guards are not
	 * absolutely necessary, but it doesn't hurt to add them anyway.
	 */
	template <Order O>
	class ThreadFunc final : private SharedState {
	public:
		using generator_t = class lib::gen::Generator<O>;
		using OutputLvl = OutputLvl::E;
	public:
		ThreadFunc(void) = delete;
		explicit ThreadFunc(SharedState s) : SharedState(s) {};
		inline void operator()(generator_t* gen, unsigned thread_num);
	private:
		trials_t trials_stop_cur_val(void) const {
			switch (trials_stop_method) {
				case StopBy::Trials:    return total_trials;
				case StopBy::Successes: return total_successes;
				default: throw "unhandled enum case";
			}
		}
	};

}
}

#endif