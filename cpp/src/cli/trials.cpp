#include "./trials.hpp"
#include "../util/ansi.hpp"
#include "./enum.hpp"

#include <iostream>
#include <iomanip>

namespace solvent::cli::trials {

const std::string THROUGHPUT_COMMENTARY =
	"\n* Throughput here is in \"average successes per operation\". Tightening the"
	"\n  threshold induces more frequent giveups, but also reduces the operational"
	"\n  cost that giveups incur. Operations are proportional to time, and machine"
	"\n  independent. The visualization bars are purposely stretched to draw focus"
	"\n  to the optimal bin. Exercise prudence against stats from small datasets!";


template<Order O>
void ThreadFunc<O>::operator()(generator_t* gen, const unsigned thread_num) {
	if (thread_num != 0) {
		// TODO [bug] old_generator can technically finish before we get here.
		generator_t const*const old_generator = gen;
		gen = new generator_t(old_generator->os);
		gen->copy_settings_from(*old_generator);
	}
	mutex.lock();
	while (true) {
		// Attempt to generate a single solution:
		mutex.unlock();
			// CRITICAL SECTION:
			// This is the only section unguarded by the mutex. That's fine
			// because it covers the overwhelming majority of the work, and
			// everything else actually accesses shared state and resources.
			gen->generate();
		mutex.lock();

		// Check break conditions:
		// Note that doing this _after_ attempting a trial (instead of before)
		// results in a tiny bit of wasted effort for the last [numThreads] or
		// so trials. I'm doing this to prevent some visual printing gaps).
		if (trials_stop_cur_val() >= trials_stop_threshold) {
			break;
		}

		// Print a progress indicator to std::cout:
		{
			using lib::gen::ExitStatus;
			ExitStatus exit_status = gen->generate_result.get_exist_status();
			const bool do_output_line =
				(output_level == OutputLvl::All) ||
				(output_level == OutputLvl::NoGiveups && exit_status == ExitStatus::Ok);
			if (total_trials % COLS == 0) {
				const unsigned new_pct_done = 100u * trials_stop_cur_val() / trials_stop_threshold;
				if (do_output_line) {
					std::cout << "\n| " << std::setw(2) << new_pct_done << "% |";
				} else if (output_level == OutputLvl::Silent) {
					const int charDiff =
						(new_pct_done * TABLE_SEPARATOR.size() / 100u)
						- (pct_done * TABLE_SEPARATOR.size() / 100u);
					for (int i = 0; i < charDiff; i++) {
						std::cout << Ansi::BLOCK_CHARS[2] << std::flush;
					}
				}
				pct_done = new_pct_done;
			}:
			total_trials++;
			if (exit_status == ExitStatus::Ok) {
				total_successes++;
			}
			// Print the generated solution:
			if (do_output_line) {
				// gen->os << '(' << thread_num << ')';
				gen->os << (gen->is_pretty ? ' ' : '\n');
				gen->print_simple();
				if constexpr (gen->O1 > 4) gen->os << std::flush;
			}
		}

		// Save some stats for later diagnostics-printing:
		const unsigned binNum = NUM_BINS * (gen->get_most_backtracks()) / gen->GIVEUP_THRESHOLD;
		bin_hit_count[binNum]++;
		bin_ops_total[binNum] += gen->generate_result.get_op_count();
	}
	mutex.unlock();
	if (thread_num != 0) delete gen;
}

}
