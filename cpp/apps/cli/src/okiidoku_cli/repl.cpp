// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku_cli/repl.hpp>

#include <okiidoku_cli/config.hpp>
#include <okiidoku_cli/timer.hpp>
#include <okiidoku_cli_utils/str.hpp>
#include <okiidoku_cli_utils/shared_rng.hpp>
#include <okiidoku/morph/canon.hpp>
#include <okiidoku/gen.hpp>
#include <okiidoku/print_2d.hpp>
// #include <okiidoku/serdes.hpp> TODO
#include <okiidoku/grid.hpp>

#include <iostream>     // cout, endl,
// #include <fstream>
// #include <filesystem>   // create_directories
#include <iomanip>      // setw,
#include <string>
#include <charconv>
#include <ctime>        // clock, CLOCKS_PER_SEC
#include <cstdint>      // uint_fastN_t
#include <system_error> // errc

namespace okiidoku::cli {

	namespace str = ::okiidoku::util::str;
	namespace {
		constexpr std::string_view welcome_message {"run `help` to get usage info"};
	}

	void Repl::start() {
		std::cout
		<< str::dim.on << welcome_message << str::dim.off
		<< std::endl;

		while (true) {
			std::cout << "\n[" << std::dec << uint_fast32_t{config_.order()} << "]: ";
			std::string command;
			std::getline(std::cin, command);
			if (!std::cin) [[unlikely]] {
				break; // input error or unrecoverable stream error.
			}
			if (std::cin.eof()) [[unlikely]] {
				break; // Ex. console <CTRL+D>
			}
			if (!run_command(command)) [[unlikely]] {
				break;
			}
		}
	}


	bool Repl::run_command(const std::string_view cmd_line) {
		const auto token_pos {cmd_line.find(' ')};
		// Very simple parsing: Assumes no leading spaces, and does not
		// trim leading or trailing spaces from the arguments substring.
		const std::string_view cmd_name {cmd_line.substr(0u, token_pos)};
		const std::string_view cmd_args {(token_pos == std::string_view::npos)
			? ""
			:  cmd_line.substr(token_pos + 1u, std::string_view::npos)}
			;
		const auto it {Command::command_str_to_enum_map.find(cmd_name)};
		if (it == Command::command_str_to_enum_map.end()) [[unlikely]] {
			// No command name was matched.
			std::cout << str::red.on << "command \"" << cmd_line << "\" not found."
				" enter \"help\" for the help menu." << str::red.off << std::endl;
			return true;
		}
		switch (it->second) {
			using Command::E;
			case E::help: [[unlikely]] {
				std::cout
				<< Command::help_message /* << str::dim.on
				important subcommand help messages can go here if needed
				<< str::dim.off */ << std::endl;
				break;
			}
			case E::quit: [[unlikely]] {
				return false;
			}
			case E::config_order: [[unlikely]] { config_.order(cmd_args); break; }
			case E::config_auto_canonicalize: [[unlikely]] { config_.canonicalize(cmd_args); break; }
			case E::gen_single: { gen_single(); break; }
			case E::gen_multiple: { gen_multiple(cmd_args); break; }
		}
		return true;
	}


	void Repl::gen_single() {
		using namespace ::okiidoku::visitor;
		const clock_t clock_start {std::clock()};
		Grid grid(config_.order());
		grid.init_most_canonical();
		shuffle(grid, shared_rng_());
		const double processor_time {(static_cast<double>(std::clock() - clock_start)) / CLOCKS_PER_SEC};
		{
			if (config_.canonicalize()) {
				canonicalize(grid); // should we make a copy and print as a second grid image?
			}
			print_2d(std::cout, shared_rng_(), grid);
		}
		std::cout << std::setprecision(4)
			<< "\nprocessor time: " << processor_time << " seconds"
			;
		std::cout << std::endl;
	}


	void Repl::gen_multiple(const std::uintmax_t how_many) {
		using namespace ::okiidoku::visitor;
		const Timer timer{};
		{
			// std::filesystem::create_directories("gen");
			const auto file_path {std::string{"gen/"} + std::to_string(uint_fast32_t{config_.order()}) + ".bin"};
			std::cout << "output file path: " << file_path << std::endl;
			// std::ofstream of(file_path, std::ios::binary|std::ios::ate);
			// TODO.wait change this to use the archiving operations.
			// try {
			// 	of.exceptions()
			// } catch (const std::ios_base::failure& fail) {
			// 	std::cout << str::red.on << fail.what() << str::red.off << std::endl;
			// }
			// alignas(std::uint_fast64_t)
			Grid grid {config_.order()};
			grid.init_most_canonical();
			const bool should_canonicalize {config_.canonicalize()};
			for (std::uintmax_t prog {0u}; prog < how_many; ++prog) {
				shuffle(grid, shared_rng_());
				if (should_canonicalize) {
					canonicalize(grid);
				}
				(void)grid;// write_solved(grid, of); // TODO this is currently crashing with floating point error :/
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

		if (elapsed.wall_seconds > 10.0) [[unlikely]] {
			// Emit a beep sound if the trials took longer than ten processor seconds:
			std::cout << '\a' << std::flush;
		}
		std::cout << std::endl;
	}


	void Repl::gen_multiple(const std::string_view how_many_str) {
		std::uintmax_t how_many {};
		if (util::str::from_chars(how_many_str, how_many).ec == std::errc{}) {
			if (how_many <= 0u) [[unlikely]] {
				std::cout << str::red.on
					<< "please provide a non-zero, positive integer."
					<< str::red.off << std::endl;
				return;
			}
		} else [[unlikely]] {
			std::cout << str::red.on
				<< "could not convert \"" << how_many_str << "\" to an integer."
				<< str::red.off << std::endl;
			return;
		}
		this->gen_multiple(how_many);
	}
}