#ifndef HPP_SOLVENT_CLI_TRIALS
#define HPP_SOLVENT_CLI_TRIALS

#include "./enum.hpp"
#include "../lib/gen/mod.hpp"

#include <mutex>
#include <string>
#include <array>

namespace solvent::cli {

using trials_t = unsigned long;

namespace trials {

	constexpr unsigned NUM_BINS = 20u;
	enum class StopAfterWhat : unsigned {
		Any,
		Ok,
	};

	const std::string TABLE_SEPARATOR = "\n+-----------+----------+----------------+-----------+-----------+";
	const std::string TABLE_HEADER    = "\n|  bin bot  |   hits   |   operations   |  giveup%  |  speedup  |";

	struct SharedData {
		std::mutex& mutex;
		const unsigned  COLS;
		const cli::verbosity::Kind output_level;
		const StopAfterWhat    trials_stop_method;
		const trials_t  stop_after;
		unsigned& pct_done;
		trials_t& total_trials;
		trials_t& total_successes;
		std::array<trials_t, NUM_BINS+1>& bin_hit_count;
		std::array<double,   NUM_BINS+1>& bin_ops_total;
	};

	/**
	 * A helper for `Repl::run_multiple`.
	 *
	 * Note: Since it is only ever used there, the include guards are not
	 * absolutely necessary, but it doesn't hurt to add them anyway.
	 */
	template<Order O>
	class ThreadFunc final : private SharedData {
	 public:
		using generator_t = class lib::gen::Generator<O>;
		using verbosity = verbosity::Kind;
	 public:
		ThreadFunc(void) = delete;
		explicit ThreadFunc(SharedData s) : SharedData(s) {};
		inline void operator()(generator_t* gen, unsigned thread_num);
	 private:
		trials_t trials_stop_cur_val(void) const {
			switch (trials_stop_method) {
				case StopAfterWhat::Any:    return total_trials;
				case StopAfterWhat::Ok: return total_successes;
				default: throw "unhandled enum case";
			}
		}
	};

}
}

#endif