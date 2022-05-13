#include <okiidoku_cli/repl.hpp>

#include <okiidoku_cli/timer.hpp>
#include <okiidoku_cli_utils/str.hpp>
#include <okiidoku/print_2d.hpp>
#include <okiidoku/morph/canon.hpp>
#include <okiidoku/gen.hpp>
#include <okiidoku/serdes.hpp>

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
		config_.order(order_input);
	}


	void Repl::start() {
		std::cout
		<< '\n' << str::dim.on << welcome_message << str::dim.off
		<< '\n' << Command::helpMessage
		<< std::endl;

		while (true) {
			std::cout << "\n[" << std::dec << config_.order() << "]: ";
			std::string command;
			std::getline(std::cin, command);
			if (!std::cin) {
				break; // input error or unrecoverable stream error.
			}
			if (std::cin.eof()) {
				break; // Ex. console <CTRL+D>
			}
			if (!run_command(command)) {
				break;
			}
		}
	}


	bool Repl::run_command(const std::string_view cmd_line) {
		const auto token_pos {cmd_line.find(' ')};
		// Very simple parsing: Assumes no leading spaces, and does not
		// trim leading or trailing spaces from the arguments substring.
		const std::string_view cmd_name {cmd_line.substr(0, token_pos)};
		const std::string_view cmd_args {(token_pos == std::string_view::npos)
			? ""
			:  cmd_line.substr(token_pos + 1, std::string_view::npos)}
			;
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
				<< Command::helpMessage /* << str::dim.on
				important subcommand help messages can go here if needed
				<< str::dim.off */ << std::endl;
				break;
			case E::quit:
				return false;
			case E::config_order:       config_.order(cmd_args); break;
			case E::config_auto_canonicalize: config_.canonicalize(cmd_args); break;
			case E::gen_single:   gen_single(); break;
			case E::gen_multiple: gen_multiple(cmd_args); break;
		}
		return true;
	}


	void Repl::gen_single() {
		using namespace okiidoku::visitor;
		const clock_t clock_start {std::clock()};
		Grid grid(config_.order());
		generate(grid, shared_rng_);
		const double processor_time {(static_cast<double>(std::clock() - clock_start)) / CLOCKS_PER_SEC};
		{
			if (config_.canonicalize()) {
				canonicalize(grid); // should we make a copy and print as a second grid image?
			}
			const auto palette_ {std::to_array<print_2d_grid_view>({
				[&](auto rmi){ return grid.at_rmi(rmi); },
			})}; // TODO.low can this just be passed inline to the printer? if not, how can we make that possible?
			print_2d(std::cout, config_.order(), palette_, shared_rng_);
		}
		std::cout << std::setprecision(4)
			<< "\nprocessor time: " << processor_time << " seconds"
			;
		std::cout << std::endl;
	}


	void Repl::gen_multiple(unsigned long long how_many) {
		using namespace okiidoku::visitor;
		const Timer timer{};
		{
			std::filesystem::create_directories("gen");
			std::string file_path {std::string{} + "gen/" + std::to_string(config_.order()) + ".bin"};
			std::cout << "output file path: " << file_path << std::endl;
			std::ofstream of(file_path, std::ios::binary|std::ios::ate);
			// TODO.high change this to use the archiving operations.
			// try {
			// 	of.exceptions()
			// } catch (const std::ios_base::failure& fail) {
			// 	std::cout << str::red.on << fail.what() << str::red.off << std::endl;
			// }
			Grid grid(config_.order());
			for (unsigned long long prog {0}; prog < how_many; ++prog) {
				generate(grid, shared_rng_);
				if (config_.canonicalize()) {
					canonicalize(grid);
				}
				// write_solution_grid_to_stream(grid, of); // TODO.high this is currently crashing with floating point error :/
				// TODO.mid print a progress bar
			}
		}
		const auto elapsed {timer.read_elapsed()};

		std::cout << std::setprecision(4) << std::dec
			<< "\nhow_many:            " << how_many
			// << "\nnum threads:        " << params.num_threads
			<< "\nprocess time (s):    " << elapsed.proc_seconds
			<< "\nwall-clock time (s): " << elapsed.wall_seconds
			;

		if (elapsed.wall_seconds > 10.0) {
			// Emit a beep sound if the trials took longer than ten processor seconds:
			std::cout << '\a' << std::flush;
		}
		std::cout << std::endl;
	}


	void Repl::gen_multiple(const std::string_view how_many_str) {
		unsigned long long how_many {};
		const auto parse_result {std::from_chars(
			how_many_str.data(), how_many_str.data()+how_many_str.size(), how_many
		)};
		if (parse_result.ec == std::errc{}) {
			if (how_many <= 0) {
				std::cout << str::red.on
					<< "please provide a non-zero, positive integer."
					<< str::red.off << std::endl;
				return;
			}
		} else {
			std::cout << str::red.on
				<< "could not convert \"" << how_many_str << "\" to an integer."
				<< str::red.off << std::endl;
			return;
		}
		this->gen_multiple(how_many);
	}
}