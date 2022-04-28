#include <okiidoku_cli/repl.hpp>

#include <okiidoku_cli/timer.hpp>
#include <okiidoku_cli_utils/str.hpp>
#include <okiidoku/fmt/print_2d.hpp>
#include <okiidoku/morph/canon.hpp>
#include <okiidoku/gen.hpp>
#include <okiidoku/db/serdes.hpp>

#include <iostream>   // cout, endl,
#include <fstream>
#include <filesystem> // create_directories
#include <iomanip>    // setw,
#include <string>
#include <charconv>

namespace okiidoku::cli {

	namespace str = okiidoku::util::str;

	constexpr std::string_view welcome_message {};


	Repl::Repl(const Order order_input, SharedRng& rng):
		shared_rng_(rng)
	{
		const Order O {order_input};
		config_.order(O);
		config_.verbosity(verbosity::E::quiet);
	}


	void Repl::start() {
		std::cout
		<< '\n' << str::dim.on << welcome_message << str::dim.off
		<< '\n' << Command::helpMessage
		<< std::endl;

		std::string command;
		do {
			std::cout << "\n[" << config_.order() << "]: ";

			std::getline(std::cin, command);
		} while (run_command(command));
	}


	bool Repl::run_command(const std::string_view cmd_line) {
		const auto token_pos {cmd_line.find(" ")};
		// Very simple parsing: Assumes no leading spaces, and does not
		// trim leading or trailing spaces from the arguments substring.
		const std::string_view cmd_name {cmd_line.substr(0, token_pos)};
		const std::string_view cmd_args {(token_pos == std::string_view::npos)
			? "" :  cmd_line.substr(token_pos + 1, std::string_view::npos)};
		const auto it {Command::enum_str_to_enum.find(cmd_name)};
		if (it == Command::enum_str_to_enum.end()) {
			// No command name was matched.
			std::cout << str::red.on << "command \"" << cmd_line << "\" not found."
				" enter \"help\" for the help menu." << str::red.off << std::endl;
			return true;
		}
		switch (it->second) {
			using Command::E;
			case E::help:
				std::cout
				<< Command::helpMessage << str::dim.on
				<< '\n' << verbosity::options_menu_str
				<< str::dim.off << std::endl;
				break;
			case E::quit:
				return false;
			case E::config_order:       config_.order(cmd_args); break;
			case E::config_print_level: config_.verbosity(cmd_args); break;
			case E::config_auto_canonicalize: config_.canonicalize(cmd_args); break;
			case E::gen_single:   gen_single(); break;
			case E::gen_multiple: gen_multiple(cmd_args); break;
		}
		return true;
	}


	void Repl::gen_single() {
		using namespace okiidoku::visitor;
		const clock_t clock_start {std::clock()};
		GridArr grid(config_.order());
		generate(grid, shared_rng_);
		const double processor_time = (static_cast<double>(std::clock() - clock_start)) / CLOCKS_PER_SEC;
		{
			if (config_.canonicalize()) {
				morph::canonicalize(grid); // should we make a copy and print as a second grid image?
			}
			const auto palette_ {std::to_array({
				print_2d_grid_view([&](auto coord){ return grid.at_row_major(coord); }),
			})}; // TODO.low can this just be passed inline to the printer? if not, how can we make that possible?
			print_2d(std::cout, config_.order(), palette_, shared_rng_);
		}
		std::cout << std::setprecision(4)
			<< "\nprocessor time: " << processor_time << " seconds"
			;
		std::cout << std::endl;
	}


	void Repl::gen_multiple(unsigned long long stop_after) {
		using namespace okiidoku::visitor;
		const Timer timer{};
		{
			std::filesystem::create_directories("gen");
			std::string file_path {std::string{} + "gen/" + std::to_string(config_.order()) + ".bin"};
			std::cout << "output file path: " << file_path << std::endl;
			std::ofstream of(file_path, std::ios::binary|std::ios::ate);
			// TODO.high change this to use the db operations.
			// try {
			// 	of.exceptions()
			// } catch (const std::ios_base::failure& fail) {
			// 	std::cout << str::red.on << fail.what() << str::red.off << std::endl;
			// }
			for (unsigned long long prog {0}; prog < stop_after; ++prog) {
				GridArr grid(config_.order());
				generate(grid, shared_rng_);
				if (config_.canonicalize()) {
					morph::canonicalize(grid);
				}
				// db::serdes::print_filled(of, grid); // TODO.high this is currently crashing with floating point error :/
				// TODO.mid print a progress bar
			}
		}
		const auto elapsed {timer.read_elapsed()};

		static const std::string seconds_units {std::string{} + str::dim.on + " seconds (with I/O)" + str::dim.off};
		std::cout << std::setprecision(4)
			<< "\nstop after:      " << stop_after
			// << "\nnum threads:     " << params.num_threads
			<< "\nprocess time:    " << elapsed.proc_seconds << seconds_units
			<< "\nwall-clock time: " << elapsed.wall_seconds << seconds_units
			// Note: the timer will not include canonicalization time if verbosity is quiet
			;

		if (elapsed.wall_seconds > 10.0) {
			// Emit a beep sound if the trials took longer than ten processor seconds:
			std::cout << '\a' << std::flush;
		}
		std::cout << std::endl;
	}


	void Repl::gen_multiple(const std::string_view stop_after_str) {
		unsigned long long stop_by_value {};
		const auto parse_result {std::from_chars(stop_after_str.begin(), stop_after_str.end(), stop_by_value)};
		if (parse_result.ec == std::errc{}) {
			if (stop_by_value <= 0) {
				std::cout << str::red.on
					<< "please provide a non-zero, positive integer."
					<< str::red.off << std::endl;
				return;
			}
		} else {
			std::cout << str::red.on
				<< "could not convert \"" << stop_after_str << "\" to an integer."
				<< str::red.off << std::endl;
			return;
		}
		this->gen_multiple(stop_by_value);
	}
}