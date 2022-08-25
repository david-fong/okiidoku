// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku_cli_utils/shared_rng.hpp>
#include <okiidoku_cli_utils/console_setup.hpp>
#include <okiidoku/gen.hpp>

#include <iostream>  // cout
#include <iomanip>   // hex
#include <charconv>
#include <string_view>
#include <random>    // random_device

/**
ARGUMENTS
1: RNG seed (default: get from device)
*/
int main(const int argc, char const *const argv[]) {
	okiidoku::util::setup_console();

	// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays, cppcoreguidelines-pro-bounds-pointer-arithmetic)
	const auto srand_key {[&]() -> std::uint_fast64_t {
		if (argc > 1) {
			const std::string_view arg {argv[1]};
			std::uint_fast64_t parsed {};
			if (std::from_chars(arg.data(), arg.data()+arg.size(), parsed, 16).ec == std::errc{}) {
				return parsed;
			}
			std::cerr << "\nfailed to parse rng seed argument (hex u64). using random_device instead.";
		}
		return std::random_device()();
	}()};
	// NOLINTEND(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays, cppcoreguidelines-pro-bounds-pointer-arithmetic)

	std::cout << "\nparsed arguments:"
	<< "\n- arg 1 (srand key)  : " << std::hex << srand_key << std::dec // TODO.mid ugh. it's using my numpunct grouping.
	<< std::endl;

	okiidoku::util::SharedRng shared_rng(srand_key);
	return 0;
}
