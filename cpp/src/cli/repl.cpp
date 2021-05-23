#include "./repl.hpp"
#include "./trials.cpp"

#include <iostream> // cout, endl,
#include <iomanip>  // setw,
#include <chrono>   // steady_clock::now, durationcast,
#include <cmath>    // pow,

#include <thread>
#include <mutex>


namespace solvent::cli {

// Mechanism to statically toggle printing alignment:
// (#undef-ed before the end of this namespace)
#define STATW_I << std::setw(this->gen.STATS_WIDTH)
#define STATW_D << std::setw(this->gen.STATS_WIDTH + 4)

const std::string TERMINAL_OUTPUT_TIPS =
"\nNote: You can run `tput rmam` in your shell to disable text wrapping."
"\nIf UTF-8 characters are garbled on Windows, run `chcp.com 65001`.";


template<Order O>
Repl<O>::Repl(std::ostream& os):
	num_extra_threads([](){
		const unsigned HWC = std::thread::hardware_concurrency();
		// HWC is specified to be zero if unknown.
		return HWC ? std::min(MAX_EXTRA_THREADS, HWC) : 0;
	}()),
	gen(os),
	os(os)
{
	set_output_level(OutputLvl::E::All);

	// Print diagnostics about Generator member size:
	std::cout
	<< "\nsolver obj size: " << sizeof(gen) << " bytes"
	<< "\ndefault genpath: " << gen.get_path_kind();
	if constexpr (O > 3) {
		std::cout << '\n' << Ansi::DIM.ON << TERMINAL_OUTPUT_TIPS << Ansi::DIM.OFF;
	}
	std::cout << std::endl;

	// Print help menu and then start the REPL (read-execute-print-loop):
	std::cout << Command::HelpMessage << std::endl;
	std::string command;
	do {
		std::cout << PROMPT;
		std::getline(std::cin, command);
	} while (run_command(command));
};


template<Order O>
bool Repl<O>::run_command(std::string const& cmd_line) {
	size_t token_pos;
	// Very simple parsing: Assumes no leading spaces, and does not
	// trim leading or trailing spaces from the arguments substring.
	const std::string cmd_name = cmd_line.substr(0, token_pos = cmd_line.find(" "));
	const std::string cmd_args = (token_pos == std::string::npos)
		? "" :  cmd_line.substr(token_pos + 1, std::string::npos);
	const auto it = Command::Str2Enum.find(cmd_name);
	if (it == Command::Str2Enum.end()) {
		// No command name was matched.
		std::cout << Ansi::RED.ON;
		std::cout << "command \"" << cmd_line << "\" not found."
			" enter \"help\" for the help menu.";
		std::cout << Ansi::RED.OFF << std::endl;
		return true;
	}
	switch (it->second) {
		using Command::E;
		case E::Help:
			std::cout
			<< Command::HelpMessage << Ansi::DIM.ON
			<< '\n' << OutputLvl::OPTIONS_MENU
			<< '\n' << lib::gen::Path::OPTIONS_MENU
			<< Ansi::DIM.OFF << std::endl;
			break;
		case E::Quit:
			return false;
		case E::OutputLevel:   set_output_level(cmd_args); break;
		case E::SetGenPath:    gen.set_path_kind(cmd_args); break;
		case E::RunSingle:     run_single();     break;
		case E::ContinuePrev:  run_single(true); break;
		case E::RunMultiple:   run_multiple(cmd_args, trials::StopBy::Trials);    break;
		case E::RunMultipleOk: run_multiple(cmd_args, trials::StopBy::Successes); break;
	}
	return true;
}


template<Order O>
OutputLvl::E Repl<O>::set_output_level(OutputLvl::E new_output_level) {
	const OutputLvl::E old_output_level = this->output_level;
	this->output_level = new_output_level;
	return old_output_level;
}


template<Order O>
OutputLvl::E Repl<O>::set_output_level(std::string const& new_output_level_str) {
	std::cout << "\noutput level is ";
	if (new_output_level_str.empty()) {
		std::cout << "currently set to: " << get_output_level() << std::endl;
		return get_output_level();
	}
	for (unsigned i = 0; i < OutputLvl::size; i++) {
		if (new_output_level_str.compare(OutputLvl::NAMES[i]) == 0) {
			if (OutputLvl::E{i} == get_output_level()) {
				std::cout << "already set to: ";
			} else {
				std::cout << "now set to: ";
				set_output_level(OutputLvl::E{i});
			}
			std::cout << get_output_level() << std::endl;
			return get_output_level();
		}
	}
	// unsuccessful return:
	std::cout << get_output_level() << " (unchanged).\n"
		<< Ansi::RED.ON << '"' << new_output_level_str
		<< "\" is not a valid output level name.\n"
		<< OutputLvl::OPTIONS_MENU << Ansi::RED.OFF << std::endl;
	return get_output_level();
}


template<Order O>
Path::E Repl<O>::set_path_kind(const Path::E new_path_kind) noexcept {
	if (new_path_kind == get_path_kind()) {
		// Short circuit:
		return get_path_kind();
	}
	const Path::E prev_path_kind = get_path_kind();
	path_kind = new_path_kind;
	return prev_path_kind;
}


template<Order O>
Path::E Repl<O>::set_path_kind(std::string const& new_path_kind_str) noexcept {
	std::cout << "\ngenerator path is ";
	if (new_path_kind_str.empty()) {
		std::cout << "currently set to: " << get_path_kind() << std::endl;
		return get_path_kind();
	}
	for (unsigned i = 0; i < Path::E_SIZE; i++) {
		if (new_path_kind_str.compare(Path::NAMES[i]) == 0) {
			if (Path::E{i} == get_path_kind()) {
				std::cout << "already set to: ";
			} else {
				std::cout << "now set to: ";
				set_path_kind(Path::E{i});
			}
			std::cout << get_path_kind() << std::endl;
			return get_path_kind();
		}
	}
	// unsuccessful return:
	std::cout << get_path_kind() << " (unchanged).\n"
		<< Ansi::RED.ON << '"' << new_path_kind_str
		<< "\" is not a valid generator path name.\n"
		<< Path::OPTIONS_MENU << Ansi::RED.OFF << std::endl;
	return get_path_kind();
}

template<Order O>
void Repl<O>::print_msg_bar(
	std::string const& msg,
	unsigned bar_length,
	const char fill_char
) const {
	if (bar_length < msg.length() + 8) {
		bar_length = msg.length() + 8;
	}
	std::string bar(bar_length, fill_char);
	if (!msg.empty()) {
		bar.replace(4, msg.length(), msg);
		bar.at(3) = ' ';
		bar.at(4 + msg.length()) = ' ';
	}
	os << '\n' <<bar;
}


template<Order O>
void Repl<O>::run_single(const bool cont_prev) {
	gen.print_msg_bar("START");

	// Generate a new solution:
	const clock_t clock_start = std::clock();
	gen.generate(cont_prev);
	const double processor_time = ((double)(std::clock() - clock_start)) / CLOCKS_PER_SEC;

	os << "\nprocessor time: " STATW_D << processor_time << " seconds";
	os << "\nnum operations: " STATW_I << gen.generate_result.get_op_count();
	os << "\nmax backtracks: " STATW_I << gen.get_most_backtracks();
	if (!gen.is_pretty) gen.print_msg_bar("", '-');
	gen.print();
	gen.print_msg_bar((gen.generate_result.get_exit_status() == lib::gen::ExitStatus::Ok) ? "DONE" : "ABORT");
	gen.generate_result = lib::gen::GenResult { exit_status: 1 }
	os << std::endl;
}


template<Order O>
void Repl<O>::run_multiple(
	const trials_t trials_stop_threshold,
	const trials::StopBy trials_stop_method
) {
	const unsigned COLS = [this](){ // Never zero. Not used when writing to file.
		const unsigned term_cols = GET_TERM_COLS();
		const unsigned cols = (term_cols-7)/(gen.O4+1);
		return term_cols ? (cols ? cols : 1) : ((unsigned[]){0,64,5,2,1,1,1})[O];
	}();
	const unsigned BAR_WIDTH = (gen.O4+1) * COLS + (gen.is_pretty ? 7 : 0);
	// Note at above: the magic number `7` is the length of the progress indicator.

	// NOTE: The last bin is for trials that do not succeed.
	std::array<trials_t, trials::NUM_BINS+1> bin_hit_count = {0,};
	std::array<double,   trials::NUM_BINS+1> bin_ops_total = {0,};

	gen.print_msg_bar("START x" + std::to_string(trials_stop_threshold), BAR_WIDTH);
	auto wall_clock_start = std::chrono::steady_clock::now();
	auto proc_clock_start = std::clock();

	trials_t total_trials = 0;
	{
		trials_t total_successes = 0u;
		unsigned percent_done = 0u;
		std::mutex shared_state_mutex;
		trials::SharedState shared_state {
			shared_state_mutex, COLS, get_output_level(),
			trials_stop_method, trials_stop_threshold, percent_done,
			total_trials, total_successes, bin_hit_count, bin_ops_total,
		};
		if (get_output_level() == OutputLvl::E::Silent) {
			std::cout << '\n';
		}

		// Start the threads:
		std::array<std::thread, MAX_EXTRA_THREADS> extra_threads;
		for (unsigned i = 0; i < num_extra_threads; i++) {
			auto thread_func = trials::ThreadFunc<O>(shared_state);
			extra_threads[i] = std::thread(std::move(thread_func), &gen, i+1);
		} {
			auto this_thread_func = trials::ThreadFunc<O>(shared_state);
			this_thread_func(&gen, 0);
		}
		for (unsigned i = 0; i < num_extra_threads; i++) {
			extra_threads[i].join();
		}
	}

	// Print stats:
	const double proc_seconds = ((double)(std::clock() - proc_clock_start) / CLOCKS_PER_SEC);
	const double wall_seconds = ((double)[wall_clock_start](){
		using namespace std::chrono;
		return duration_cast<microseconds>(steady_clock::now() - wall_clock_start);
	}().count() / 1'000'000);
	const std::string seconds_units = DIM_ON + " seconds (with I/O)" + DIM_OFF;
	gen.print_msg_bar("", BAR_WIDTH, '-'); os
	<< "\nhelper threads: " STATW_I << num_extra_threads
	<< "\ngenerator path: " STATW_I << gen.get_path_kind()
	// TODO [stats] For total successes and total trieals.
	<< "\nprocessor time: " STATW_D << proc_seconds << seconds_units
	<< "\nreal-life time: " STATW_D << wall_seconds << seconds_units;
	;
	if (wall_seconds > 10.0) {
		// Emit a beep sound if the trials took longer than ten processor seconds:
		std::cout << '\a' << std::flush;
	}
	// Print bins (work distribution):
	print_trials_work_distribution(total_trials, bin_hit_count, bin_ops_total);
	gen.print_msg_bar("DONE x" + std::to_string(trials_stop_threshold), BAR_WIDTH);
	os << std::endl;
}


template<Order O>
void Repl<O>::run_multiple(
	std::string const& trials_string,
	const trials::StopBy stop_by_method
) {
	long stopByValue;
	try {
		stopByValue = std::stol(trials_string);
		if (stopByValue <= 0) {
			std::cout << Ansi::RED.ON;
			std::cout << "please provide a non-zero, positive integer.";
			std::cout << Ansi::RED.OFF << std::endl;
			return;
		}
	} catch (std::invalid_argument const& ia) {
		std::cout << Ansi::RED.ON;
		std::cout << "could not convert \"" << trials_string << "\" to an integer.";
		std::cout << Ansi::RED.OFF << std::endl;
		return;
	}
	run_multiple(static_cast<trials_t>(stopByValue), stop_by_method);
}


template<Order O>
void Repl<O>::print_trials_work_distribution(
	const trials_t total_trials, // sum of entries of bin_hit_count
	std::array<trials_t, trials::NUM_BINS+1> const& bin_hit_count,
	std::array<double,   trials::NUM_BINS+1> const& bin_ops_total
) {
	const std::string THROUGHPUT_BAR_STRING = "--------------------------------";

	// Calculate all throughputs before printing:
	// (done in its own loop so we can later print comparisons against the optimal bin)
	std::array<double, trials::NUM_BINS+1> throughput;
	std::array<double, trials::NUM_BINS+1> successful_trials_accum_arr;
	unsigned  best_throughput_bin    = 0u; {
	opcount_t successful_trials_accum = 0u;
	double  successful_solve_ops_accum = 0.0;
	for (unsigned i = 0; i < trials::NUM_BINS; i++) {
		successful_trials_accum   += bin_hit_count[i];
		successful_solve_ops_accum += bin_ops_total[i];
		successful_trials_accum_arr[i] = successful_trials_accum;
		const double bounded_giveup_ops = ((double)bin_ops_total[i+1] / bin_hit_count[i+1]);
			// No nice way to do the above. If I want an exact thing, I would
			// need to change generate to also track the numOperations
			// for some hypothetical, lower threshold, which would be for the
			// bottom of this bin. I would need to expose `trials::NUM_BINS` to
			// the `Generator` class. As a temporary, pessimistic band-aid, I will
			// use the values for the next bin. Note that this will give `nan`
			// (0.0/0.0) if there is no data for the next bin.
		const double bounded_giveup_ops_total = (total_trials - successful_trials_accum) * bounded_giveup_ops;
		throughput[i] = successful_trials_accum / (successful_solve_ops_accum + bounded_giveup_ops_total);
		if (throughput[i] > throughput[best_throughput_bin]) {
			best_throughput_bin = i;
		}
	}}
	throughput[trials::NUM_BINS] = 0.0; // unknown.
	successful_trials_accum_arr[trials::NUM_BINS] = 0.0;

	os << trials::TABLE_SEPARATOR;
	os << trials::TABLE_HEADER;
	os << trials::TABLE_SEPARATOR;
	for (unsigned i = 0; i < bin_hit_count.size(); i++) {
		if (i == trials::NUM_BINS) {
			// Print a special separator for the giveups row:
			os << trials::TABLE_SEPARATOR;
		}
		// Bin Bottom column:
		const double bin_bottom  = (double)(i) * gen.GIVEUP_THRESHOLD / trials::NUM_BINS;
		if constexpr (O <= 4) {
			os << "\n|" << std::setw(9) << (int)(bin_bottom);
		} else {
			os << "\n|" << std::setw(8) << (int)(bin_bottom / 1'000.0) << 'K';
		}
		// Bin Hit Count column:
		os << "  |";
		if (bin_hit_count[i] == 0) os << DIM_ON;
		os << std::setw(8) << bin_hit_count[i];
		if (bin_hit_count[i] == 0) os << DIM_OFF;

		// Operation Count column:
		os << "  |";
		if (bin_hit_count[i] == 0) os << DIM_ON;
		os << std::setw(13) << unsigned(bin_ops_total[i] / ((O<5)?1:1000));
		os << ((O<5)?' ':'K');
		if (bin_ops_total[i] == 0) os << DIM_OFF;

		// Giveup Percentage column:
		os << "  |";
		os << std::setw(9) << (100.0 * (total_trials - successful_trials_accum_arr[i]) / total_trials);

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
		const unsigned bar_length = THROUGHPUT_BAR_STRING.length()
			* std::pow(throughput[i] / throughput[best_throughput_bin], static_cast<int>(20.0/O));
		if (i != best_throughput_bin) os << DIM_ON;
		os << ' ' << THROUGHPUT_BAR_STRING.substr(0, bar_length);
		if (i != best_throughput_bin) os << DIM_OFF;
	}
	os << " <- current giveup threshold";
	os << trials::TABLE_SEPARATOR;
	os << DIM_ON << trials::THROUGHPUT_COMMENTARY << DIM_OFF;
}

#undef STATW_I
#undef STATW_D

}
