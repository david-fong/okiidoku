// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku_cli/repl.hpp>

#include <okiidoku_cli_utils/shared_rng.hpp>
#include <okiidoku_cli_utils/console_setup.hpp>
#include <okiidoku_cli_utils/str.hpp>
#include <okiidoku/ints_io.hpp>
#include <okiidoku/order.hpp>
#include <okiidoku/about.hpp>

#include <iostream>     // cout
#include <ios>          // hex
#include <charconv>
#include <string>       // stoi
#include <string_view>
#include <array>
#include <random>       // random_device
#include <cstdint>      // uint..._t
#include <system_error> // errc

/**
ARGUMENTS
1: grid order
2: RNG seed (default: get from device)
*/
int main(const int argc, char const *const argv[]) {
	namespace ok = ::okiidoku;
	auto* numpunct {ok::util::setup_console()};

	#if defined(__clang__) or defined(__EMSCRIPTEN__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
	#endif
	// NOLINTBEGIN(*-avoid-c-arrays, cppcoreguidelines-pro-bounds-pointer-arithmetic)
	const auto user_order {(argc > 1)
		? static_cast<ok::Order>(std::stoi(argv[1uz]))
		: ok::compiled_orders.front()
	};
	const auto srand_key {[&]() -> std::uint_fast64_t {
		if (argc > 2) {
			const std::string_view arg {argv[2uz]};
			std::uint_fast64_t parsed {}; // NOLINT(misc-const-correctness) seems like a clang-tidy bug :/
			if (ok::util::str::from_chars(arg, parsed, 16u).ec == std::errc{}) {
				return parsed;
			}
			std::cerr << "\nfailed to parse rng seed argument (hex u64). using random_device instead.";
		}
		return std::random_device{}();
	}()};
	// NOLINTEND(*-avoid-c-arrays, cppcoreguidelines-pro-bounds-pointer-arithmetic)
	#if defined(__clang__) or defined(__EMSCRIPTEN__)
	#pragma GCC diagnostic pop
	#endif

	std::cout << "\nparsed arguments:"
		<< "\n- arg 1 (grid order) : " << user_order
		<< "\n- arg 2 (srand key)  : " << std::hex;
			numpunct->set_grouping(0); std::cout << srand_key;
			numpunct->set_grouping(3); std::cout << std::dec;
	std::cout << std::endl;

	ok::util::SharedRng shared_rng {srand_key};

	ok::cli::Repl repl {user_order, shared_rng};
	repl.start();

	std::cout << "\nbye bye!\n\n";
	return 0;
}
