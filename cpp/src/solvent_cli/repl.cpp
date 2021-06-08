#include <solvent_cli/repl.hpp>

#include <solvent_lib/print.hpp>
#include <solvent_util/timer.hpp>
#include <solvent_util/ansi.hpp>

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
		set_path_kind(pathkind_t::RowMajor);
	}

	template<Order O>
	void Repl<O>::start(void) {
		std::cout
		<< '\n' << ansi::DIM.ON << TERMINAL_OUTPUT_TIPS << ansi::DIM.OFF
		<< '\n' << Command::HelpMessage
		<< std::endl;

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
	void Repl<O>::print_msg_bar(std::string const& msg, const char fill_char) const {
		return print_msg_bar(msg, 64, fill_char);
	}


	template<Order O>
	void Repl<O>::gen_single(const bool cont_prev) {
		print_msg_bar("START");

		// Generate a new solution:
		generator_t gen;
		std::cout << "\nsolver obj size: " << sizeof(gen) << " bytes";
		const clock_t clock_start = std::clock();
		const auto& gen_result = cont_prev
			? gen.generate(std::nullopt)
			: gen.generate(gen::Params{.path_kind = path_kind_});
		const double processor_time = (static_cast<double>(std::clock() - clock_start)) / CLOCKS_PER_SEC;

		std::cout << "\nprocessor time: " STATW_D << processor_time << " seconds";
		std::cout << "\nnum operations: " STATW_I << gen_result.op_count;
		std::cout << "\nmax backtracks: " STATW_I << gen_result.most_backtracks_seen;
		print_msg_bar("", '-');
		print::pretty<O>(std::cout, gen_result);
		print_msg_bar((gen_result.status == gen::ExitStatus::Ok) ? "OK" : "ABORT");
		std::cout << std::endl;
	}


	template<Order O>
	void Repl<O>::gen_multiple(
		const trials_t stop_after,
		const bool only_count_oks
	) {
		const unsigned BAR_WIDTH = 64;

		print_msg_bar("START x" + std::to_string(stop_after), BAR_WIDTH);
		gen::batch::Params params {
			.gen_params { .path_kind = path_kind_ },
			.only_count_oks = only_count_oks,
			.stop_after = stop_after
		};
		const gen::batch::BatchReport batch_report = gen::batch::batch<O>(params,
			[this](typename generator_t::GenResult const& gen_result) {
				if ((verbosity_ == verbosity::Kind::All)
				 || ((verbosity_ == verbosity::Kind::NoGiveups) && (gen_result.status == gen::ExitStatus::Ok))
				) {
					print::serial<O>(std::cout, gen_result);
					if constexpr (O > 4) {
						std::cout << std::endl;
					} else {
						std::cout << '\n';
					}
				} else if (verbosity_ == verbosity::Kind::Silent) {
					// TODO.impl print a progress bar
				}
			}
		);
		print_msg_bar("", BAR_WIDTH, '-');

		const std::string seconds_units = DIM_ON + " seconds (with I/O)" + DIM_OFF;
		std::cout
			<< "\nhelper threads: " STATW_I << params.num_threads
			<< "\ngenerator path: " STATW_I << params.gen_params.path_kind
			// TODO [stats] For total successes and total trials.
			<< "\nprocessor time: " STATW_D << batch_report.time_elapsed.proc_seconds << seconds_units
			<< "\nreal-life time: " STATW_D << batch_report.time_elapsed.wall_seconds << seconds_units
			;

		// Print bins (work distribution):
		batch_report.print(std::cout, O);
		print_msg_bar("DONE x" + std::to_string(stop_after), BAR_WIDTH);

		if (batch_report.time_elapsed.wall_seconds > 10.0) {
			// Emit a beep sound if the trials took longer than ten processor seconds:
			std::cout << '\a' << std::flush;
		}
		std::cout << std::endl;
	}


	template<Order O>
	void Repl<O>::gen_multiple(
		std::string const& stop_after_str,
		const bool only_count_oks
	) {
		unsigned long stopByValue;
		try {
			stopByValue = std::stoul(stop_after_str);
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


	#define SOLVENT_TEMPL_TEMPL(O_) \
		template class Repl<O_>;
	SOLVENT_INSTANTIATE_ORDER_TEMPLATES
	#undef SOLVENT_TEMPL_TEMPL

	#undef STATW_I
	#undef STATW_D
}