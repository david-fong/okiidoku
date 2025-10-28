// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_CLI_REPL
#define HPP_OKIIDOKU_CLI_REPL

#include <okiidoku_cli/config.hpp>
#include <okiidoku_cli_utils/shared_rng.hpp>
#include <okiidoku/order.hpp>

#include <map>
#include <string_view>
#include <utility> // iwyu says this is for std::pair??
#include <cstdint>

namespace okiidoku::cli {

	namespace Command {
		enum class E : unsigned char {
			help,
			quit,
			config_order,
			config_auto_canonicalize,
			gen_single,
			gen_multiple,
		};
		inline const std::map<std::string_view, Command::E> command_str_to_enum_map {
			{ "help",  E::help },
			{ "quit",  E::quit },
			{ "order", E::config_order },
			{ "canon", E::config_auto_canonicalize },
			{ "",      E::gen_single },
			{ "gen",   E::gen_multiple },
		};
		inline constexpr std::string_view help_message {"\nCOMMAND MENU:"
		"\n- help               print this help menu"
		"\n- quit               cleanly exit this program"
		"\n"
		"\n- order [<order>]    get/set order (sqrt of grid length)"
		"\n- canon [<y/n>]      get/set canonicalization"
		"\n"
		"\n- {enter}            generate a single solution"
		"\n- gen <n>            generate <n> solutions"
		};
	}


	class Repl final {
	public:
		explicit Repl(const Order order_input, ::okiidoku::util::Prng prng):
			prng_ {prng}
		{
			config_.order(order_input);
		}

		void start();

		/** \return `false` if command requested to end REPL. */
		[[nodiscard]] bool run_command(std::string_view cmd_line);

	private:
		::okiidoku::util::Prng prng_;
		Config config_;

		void gen_single();

		void gen_multiple(std::uintmax_t how_many);
		void gen_multiple(std::string_view how_many_str);
	};
}
#endif