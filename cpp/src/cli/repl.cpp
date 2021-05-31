#include "./repl.hpp"

#include ":/lib/gen/batch.hpp"
#include ":/lib/print.hpp"
#include ":/util/timer.hpp"
#include ":/util/ansi.hpp"

#include <iostream> // cout, endl,
#include <iomanip>  // setw,
#include <cmath>    // pow,

namespace solvent::cli {

	namespace ansi = solvent::util::ansi;
	using namespace solvent::lib;

	using pathkind_t = lib::gen::path::Kind;

	// Mechanism to statically toggle printing alignment:
	// (#undef-ed before the end of this namespace)
	#define STATW_I << std::setw((0.4 * O*O * 4))
	#define STATW_D << std::setw((0.4 * O*O * 4) + 4)

	const std::string TERMINAL_OUTPUT_TIPS =
	"\nNote: You can run `tput rmam` in your shell to disable text wrapping."
	"\nIf UTF-8 characters are garbled on Windows, run `chcp.com 65001`.";


	template<Order O>
	Repl<O>::Repl() {
		set_verbosity(verbosity::Kind::All);
	}

	template<Order O>
	void Repl<O>::start(void) {
		// Print diagnostics about Generator member size:
		std::cout
		<< "\nsolver obj size: " << sizeof(gen_) << " bytes"
		<< '\n' << ansi::DIM.ON << TERMINAL_OUTPUT_TIPS << ansi::DIM.OFF;
		std::cout << std::endl;

		// Print help menu and then start the REPL (read-execute-print-loop):
		std::cout << Command::HelpMessage << std::endl;
		std::string command;
		do {
			std::cout << PROMPT;
			std::getline(std::cin, command);
		} while (run_command(command));
	}


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
			std::cout << ansi::RED.ON;
			std::cout << "command \"" << cmd_line << "\" not found."
				" enter \"help\" for the help menu.";
			std::cout << ansi::RED.OFF << std::endl;
			return true;
		}
		switch (it->second) {
			using Command::E;
			case E::Help:
				std::cout
				<< Command::HelpMessage << ansi::DIM.ON
				<< '\n' << verbosity::OPTIONS_MENU
				<< '\n' << gen::path::OPTIONS_MENU
				<< ansi::DIM.OFF << std::endl;
				break;
			case E::Quit:
				return false;
			case E::ConfigVerbosity:  set_verbosity(cmd_args); break;
			case E::ConfigGenPath:    set_path_kind(cmd_args); break;
			case E::GenSingle:     gen_single();     break;
			case E::GenContinue:   gen_single(true); break;
			case E::GenMultiple:   gen_multiple(cmd_args, false); break;
			case E::GenMultipleOk: gen_multiple(cmd_args, true); break;
		}
		return true;
	}


	template<Order O>
	verbosity::Kind Repl<O>::set_verbosity(verbosity::Kind new_output_level) {
		const verbosity::Kind old_output_level = this->verbosity_;
		this->verbosity_ = new_output_level;
		return old_output_level;
	}


	template<Order O>
	verbosity::Kind Repl<O>::set_verbosity(std::string const& new_output_level_str) {
		std::cout << "\noutput level is ";
		if (new_output_level_str.empty()) {
			std::cout << "currently set to: " << get_verbosity() << std::endl;
			return get_verbosity();
		}
		for (unsigned i = 0; i < verbosity::size; i++) {
			if (new_output_level_str.compare(verbosity::NAMES[i]) == 0) {
				if (verbosity::Kind{i} == get_verbosity()) {
					std::cout << "already set to: ";
				} else {
					std::cout << "now set to: ";
					set_verbosity(verbosity::Kind{i});
				}
				std::cout << get_verbosity() << std::endl;
				return get_verbosity();
			}
		}
		// unsuccessful return:
		std::cout << get_verbosity() << " (unchanged).\n"
			<< ansi::RED.ON << '"' << new_output_level_str
			<< "\" is not a valid output level name.\n"
			<< verbosity::OPTIONS_MENU << ansi::RED.OFF << std::endl;
		return get_verbosity();
	}


	template<Order O>
	const pathkind_t Repl<O>::set_path_kind(const pathkind_t new_path_kind) noexcept {
		if (new_path_kind == get_path_kind()) {
			// Short circuit:
			return get_path_kind();
		}
		const pathkind_t prev_path_kind = get_path_kind();
		path_kind_ = new_path_kind;
		return prev_path_kind;
	}


	template<Order O>
	const pathkind_t Repl<O>::set_path_kind(std::string const& new_path_kind_str) noexcept {
		std::cout << "\ngenerator path is ";
		if (new_path_kind_str.empty()) {
			std::cout << "currently set to: " << get_path_kind() << std::endl;
			return get_path_kind();
		}
		for (unsigned i = 0; i < gen::path::NUM_KINDS; i++) {
			if (new_path_kind_str.compare(gen::path::NAMES[i]) == 0) {
				if (pathkind_t{i} == get_path_kind()) {
					std::cout << "already set to: ";
				} else {
					std::cout << "now set to: ";
					set_path_kind(pathkind_t{i});
				}
				std::cout << get_path_kind() << std::endl;
				return get_path_kind();
			}
		}
		// unsuccessful return:
		std::cout << get_path_kind() << " (unchanged).\n"
			<< ansi::RED.ON << '"' << new_path_kind_str
			<< "\" is not a valid generator path name.\n"
			<< gen::path::OPTIONS_MENU << ansi::RED.OFF << std::endl;
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
		std::cout << '\n' <<bar;
	}


	template<Order O>
	void Repl<O>::gen_single(const bool cont_prev) {
		print_msg_bar("START");

		// Generate a new solution:
		const clock_t clock_start = std::clock();
		const auto& gen_result = cont_prev
			? gen_.generate(std::nullopt)
			: gen_.generate(gen::Params{.path_kind = path_kind_});
		const double processor_time = (static_cast<double>(std::clock() - clock_start)) / CLOCKS_PER_SEC;

		std::cout << "\nprocessor time: " STATW_D << processor_time << " seconds";
		std::cout << "\nnum operations: " STATW_I << gen_result.op_count;
		std::cout << "\nmax backtracks: " STATW_I << gen_result.most_backtracks_seen;
		print_msg_bar("", '-');
		print::pretty(std::cout, O, gen_result);
		print_msg_bar((gen_result.exit_status == gen::ExitStatus::Ok) ? "OK" : "ABORT");
		std::cout << std::endl;
	}


	template<Order O>
	void Repl<O>::gen_multiple(
		const trials_t stop_after,
		const bool only_count_oks
	) {
		const unsigned COLS = [this](){ // Never zero. Not used when writing to file.
			const unsigned term_cols = GET_TERM_COLS();
			const unsigned cols = (term_cols-7)/(gen_.O4+1);
			return term_cols ? (cols ? cols : 1) : [](){ const unsigned _[] = {0,64,5,2,1,1,1}; return _[O]; }();
		}();
		const unsigned BAR_WIDTH = (gen_.O4+1) * COLS + (7);
		// Note at above: the magic number `7` is the length of the progress indicator.

		print_msg_bar("START x" + std::to_string(stop_after), BAR_WIDTH);
		gen::batch::Params params {
			.gen_params { .path_kind = path_kind_ },
			.only_count_oks = only_count_oks,
			.stop_after = stop_after
		};
		const auto batch_report = gen::batch::batch<O>(params, [](typename gen::Generator<O>::GenResult const& gen_result){
			print::serial(std::count, O, gen_result);
			std::cout << std::endl;
		});

		const std::string seconds_units = DIM_ON + " seconds (with I/O)" + DIM_OFF;
		print_msg_bar("", BAR_WIDTH, '-'); std::cout
		<< "\nhelper threads: " STATW_I << params.num_threads
		<< "\ngenerator path: " STATW_I << params.gen_params.path_kind
		// TODO [stats] For total successes and total trials.
		<< "\nprocessor time: " STATW_D << batch_report.time_elapsed.proc_seconds << seconds_units
		<< "\nreal-life time: " STATW_D << batch_report.time_elapsed.wall_seconds << seconds_units;

		if (batch_report.time_elapsed.wall_seconds > 10.0) {
			// Emit a beep sound if the trials took longer than ten processor seconds:
			std::cout << '\a' << std::flush;
		}
		// Print bins (work distribution):
		print_trials_work_distribution(batch_report);
		print_msg_bar("DONE x" + std::to_string(stop_after), BAR_WIDTH);
		std::cout << std::endl;
	}


	template<Order O>
	void Repl<O>::gen_multiple(
		std::string const& stop_after_str,
		const bool only_count_oks
	) {
		long stopByValue;
		try {
			stopByValue = std::stol(stop_after_str);
			if (stopByValue <= 0) {
				std::cout << ansi::RED.ON;
				std::cout << "please provide a non-zero, positive integer.";
				std::cout << ansi::RED.OFF << std::endl;
				return;
			}
		} catch (std::invalid_argument const& ia) {
			std::cout << ansi::RED.ON;
			std::cout << "could not convert \"" << stop_after_str << "\" to an integer.";
			std::cout << ansi::RED.OFF << std::endl;
			return;
		}
		this->gen_multiple(static_cast<trials_t>(stopByValue), only_count_oks);
	}


	template<Order O>
	void Repl<O>::print_trials_work_distribution(
		gen::batch::Params const& params,
		gen::batch::BatchReport const& batch_report
	) {
		static const std::string THROUGHPUT_BAR_STRING("--------------------------------");
		static const std::string TABLE_SEPARATOR = "\n+------------------+------------------+------------------+-------------------+";
		static const std::string TABLE_HEADER    = "\n|  max backtracks  |   marginal oks   |   marginal ops   |  net average ops  |";

		std::cout << TABLE_SEPARATOR;
		std::cout << TABLE_HEADER;
		std::cout << TABLE_SEPARATOR;
		unsigned best_sample_i = 0;
		for (unsigned sample_i = 0; sample_i < batch_report.max_backtrack_samples.size(); sample_i++) {
			const auto& sample = batch_report.max_backtrack_samples[sample_i];

			// max_backtracks:
			if constexpr (O <= 4) {
				std::cout << "\n|" << std::setw(9) << sample.max_backtracks;
			} else {
				std::cout << "\n|" << std::setw(8) << (sample.max_backtracks / 1'000.0) << 'K';
			}
			// marginal_oks:
			std::cout << "  |";
			if (sample.marginal_oks == 0) std::cout << DIM_ON;
			std::cout << std::setw(8) << sample.marginal_oks;
			if (sample.marginal_oks == 0) std::cout << DIM_OFF;

			// marginal_ops:
			std::cout << "  |";
			if (sample.marginal_oks == 0) std::cout << DIM_ON;
			std::cout << std::setw(13) << static_cast<unsigned>(sample.marginal_ops / ((O<5)?1:1000));
			std::cout << ((O<5)?' ':'K');
			if (sample.marginal_oks == 0) std::cout << DIM_OFF;

			// net_average_ops:
			std::cout << "  |";
			std::cout << std::setw(9) << (100.0 * sample.net_average_ops);

			// Closing right-edge:
			std::cout << "  |";

			// Print a bar to visualize throughput relative to tha
			// of the best. Note visual exaggeration via exponents
			// (the exponent value was chosen by taste / visual feel)
			const unsigned bar_length = THROUGHPUT_BAR_STRING.length() * std::pow(
				sample.net_average_ops / batch_report.max_backtrack_samples[best_sample_i].net_average_ops,
				static_cast<int>(20.0/O)
			);
			if (sample_i != best_sample_i) std::cout << DIM_ON;
			std::cout << ' ' << THROUGHPUT_BAR_STRING.substr(0, bar_length);
			if (sample_i != best_sample_i) std::cout << DIM_OFF;
		}
		std::cout << TABLE_SEPARATOR;
		if (batch_report.total_oks < params.max_backtrack_sample_granularity * gen::batch::SharedData::RECOMMENDED_OKS_PER_SAMPLE) {
			std::cout << DIM_ON << "\nexercise caution against small datasets!\n" << DIM_OFF;
		}
	}

	#undef STATW_I
	#undef STATW_D
}

template class solvent::cli::Repl<4>;