#include <solvent_cli/repl.hpp>

#include <solvent_lib/print.hpp>
#include <solvent_util/timer.hpp>
#include <solvent_util/str.hpp>

#include <iostream> // cout, endl,
#include <iomanip>  // setw,

namespace solvent::cli {

	namespace str = solvent::util::str;
	using namespace solvent::lib;

	using pathkind_t = lib::gen::path::Kind;

	// Mechanism to statically toggle printing alignment:
	// (#undef-ed before the end of this namespace)
	#define STATW_I << std::setw((0.4 * config_.order() * config_.order() * 1.5))
	#define STATW_D << std::setw((0.4 * config_.order() * config_.order() * 1.5)) << std::fixed << std::setprecision(2)

	const std::string TERMINAL_OUTPUT_TIPS =
	"\nNote: You can run `tput rmam` in your shell to disable text wrapping."
	"\nIf UTF-8 characters are garbled on Windows, run `chcp.com 65001`.";


	Repl::Repl(const Order O) {
		config_.order(O);
		if (O <= 4) { config_.verbosity(verbosity::Kind::Silent); }
		if (O  > 4) { config_.verbosity(verbosity::Kind::NoGiveups); }
		config_.path_kind(pathkind_t::RowMajor);
		config_.max_dead_ends(0);
	}


	void Repl::start(void) {
		const auto my_numpunct = new util::str::MyNumPunct;
		const auto pushed_locale = std::cout.imbue(std::locale(std::cout.getloc(), my_numpunct ));
		std::cout
		<< '\n' << str::DIM.ON << TERMINAL_OUTPUT_TIPS << str::DIM.OFF
		<< '\n' << Command::HelpMessage
		<< std::endl;

		std::string command;
		do {
			std::cout << "\n[" << config_.order() << "]: ";

			std::getline(std::cin, command);
		} while (run_command(command));
		std::cout.imbue(pushed_locale);
		// delete my_numpunct; // TODO.learn why isn't this needed?
	}


	bool Repl::run_command(std::string const& cmd_line) {
		size_t token_pos;
		// Very simple parsing: Assumes no leading spaces, and does not
		// trim leading or trailing spaces from the arguments substring.
		const std::string cmd_name = cmd_line.substr(0, token_pos = cmd_line.find(" "));
		const std::string cmd_args = (token_pos == std::string::npos)
			? "" :  cmd_line.substr(token_pos + 1, std::string::npos);
		const auto it = Command::Str2Enum.find(cmd_name);
		if (it == Command::Str2Enum.end()) {
			// No command name was matched.
			std::cout << str::RED.ON;
			std::cout << "command \"" << cmd_line << "\" not found."
				" enter \"help\" for the help menu.";
			std::cout << str::RED.OFF << std::endl;
			return true;
		}
		switch (it->second) {
			using Command::E;
			case E::Help:
				std::cout
				<< Command::HelpMessage << str::DIM.ON
				<< '\n' << verbosity::OPTIONS_MENU
				<< '\n' << gen::path::OPTIONS_MENU
				<< str::DIM.OFF << std::endl;
				break;
			case E::Quit:
				return false;
			case E::ConfigOrder:       config_.order(cmd_args); break;
			case E::ConfigVerbosity:   config_.verbosity(cmd_args); break;
			case E::ConfigGenPath:     config_.path_kind(cmd_args); break;
			case E::ConfigMaxDeadEnds: config_.max_dead_ends(cmd_args); break;
			case E::GenSingle:     gen_single();     break;
			case E::GenContinue:   gen_single(true); break;
			case E::GenMultiple:   gen_multiple(cmd_args, false); break;
			case E::GenMultipleOk: gen_multiple(cmd_args, true); break;
		}
		return true;
	}


	void Repl::gen_single(const bool cont_prev) {
		str::print_msg_bar("START");

		// Generate a new solution:
		const clock_t clock_start = std::clock();
		const auto gen_result = cont_prev
			? toolkit.gen_continue_prev(config_.order())
			: toolkit.gen(config_.order(), gen::Params{
				.path_kind = config_.path_kind(),
				.max_dead_ends = config_.max_dead_ends(),
			}); // TODO.fix previous generator needs to be persisted.
		const double processor_time = (static_cast<double>(std::clock() - clock_start)) / CLOCKS_PER_SEC;

		std::cout << "\nprocessor time: " STATW_D << processor_time << " seconds";
		std::cout << "\nnum operations: " STATW_I << gen_result.op_count;
		std::cout << "\nmax dead ends:  " STATW_I << gen_result.most_dead_ends_seen;
		str::print_msg_bar("", "─");
		gen_result.print_pretty(std::cout);
		str::print_msg_bar((gen_result.status == gen::ExitStatus::Ok) ? "OK" : "ABORT");
		std::cout << std::endl;
	}


	void Repl::gen_multiple(
		const trials_t stop_after,
		const bool only_count_oks
	) {
		const unsigned BAR_WIDTH = 64;

		str::print_msg_bar("START x" + std::to_string(stop_after), BAR_WIDTH);
		gen::batch::Params params {
			.gen_params { .path_kind = config_.path_kind(), .max_dead_ends = config_.max_dead_ends() },
			.only_count_oks = only_count_oks,
			.stop_after = stop_after
		};
		const gen::batch::BatchReport batch_report = gen::batch::batch(config_.order(), params,
			[this](gen::GenResult const& gen_result) {
				if ((config_.verbosity() == verbosity::Kind::All)
				 || ((config_.verbosity() == verbosity::Kind::NoGiveups) && (gen_result.status == gen::ExitStatus::Ok))
				) {
					gen_result.print_serial(std::cout);
					if (config_.order() <= 4) {
						std::cout << '\n';
					} else {
						std::cout << std::endl; // always flush for big grids
					}
				} else if (config_.verbosity() == verbosity::Kind::Silent) {
					// TODO.impl print a progress bar
				}
			}
		);
		if (config_.verbosity() == verbosity::Kind::NoGiveups
			&& only_count_oks
			&& batch_report.total_anys == 0
		) {
			std::cout << str::RED.ON << "* all generations aborted" << str::RED.OFF;
		}
		if (config_.verbosity() != verbosity::Kind::Silent) {
			str::print_msg_bar("", BAR_WIDTH, "─");
		}

		static const std::string seconds_units = std::string() + str::DIM.ON + " seconds (with I/O)" + str::DIM.OFF;
		std::cout
			<< "\nhelper threads: " STATW_I << params.num_threads
			<< "\ngenerator path: " STATW_I << params.gen_params.path_kind
			<< "\npercent aborts: " STATW_D << (batch_report.fraction_aborted * 100) << " %"
			<< "\nprocessor time: " STATW_D << batch_report.time_elapsed.proc_seconds << seconds_units
			<< "\nreal-life time: " STATW_D << batch_report.time_elapsed.wall_seconds << seconds_units
			;

		// Print bins (work distribution):
		batch_report.print(std::cout, config_.order());
		str::print_msg_bar("DONE x" + std::to_string(stop_after), BAR_WIDTH);

		if (batch_report.time_elapsed.wall_seconds > 10.0) {
			// Emit a beep sound if the trials took longer than ten processor seconds:
			std::cout << '\a' << std::flush;
		}
		std::cout << std::endl;
	}


	void Repl::gen_multiple(
		std::string const& stop_after_str,
		const bool only_count_oks
	) {
		unsigned long stop_by_value;
		try {
			stop_by_value = std::stoul(stop_after_str);
			if (stop_by_value <= 0) {
				std::cout << str::RED.ON;
				std::cout << "please provide a non-zero, positive integer.";
				std::cout << str::RED.OFF << std::endl;
				return;
			}
		} catch (std::invalid_argument const& ia) {
			std::cout << str::RED.ON;
			std::cout << "could not convert \"" << stop_after_str << "\" to an integer.";
			std::cout << str::RED.OFF << std::endl;
			return;
		}
		this->gen_multiple(static_cast<trials_t>(stop_by_value), only_count_oks);
	}


	#undef STATW_I
	#undef STATW_D
}