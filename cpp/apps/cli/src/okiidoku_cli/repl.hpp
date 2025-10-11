// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_CLI_REPL
#define HPP_OKIIDOKU_CLI_REPL

#include <okiidoku_cli/config.hpp>
#include <okiidoku/order.hpp>
namespace okiidoku::util { class SharedRng; }

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
		// TODO: consider taking ownership of rng? or if we can get a space-cheap implementation, doesn't matter
		explicit Repl(const Order order_input, util::SharedRng& rng):
			shared_rng_(rng)
		{
			config_.order(order_input);
		}

		// disallow copies and moves:
		Repl(const Repl&) = delete;
		Repl& operator=(const Repl&) = delete;
		// Note to self: move operations are not implicitly declared if copy operations are user-declared.
		// TODO pretty sure I only did this because rng is a reference. would wrapping cref or non-owning smart pointer type help? or just switching to a PRNG with good characteristics and small state size... (PCG?)

		void start();
		bool run_command(std::string_view cmd_line);

	private:
		util::SharedRng& shared_rng_;
		Config config_;

		void gen_single();

		void gen_multiple(std::uintmax_t how_many);
		void gen_multiple(std::string_view how_many_str);
	};
}
#endif