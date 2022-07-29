#include <okiidoku_cli/repl.hpp>

#include <okiidoku_cli_utils/shared_rng.hpp>
#include <okiidoku_cli_utils/console_setup.hpp>

#include <iostream>  // cout
#include <iomanip>   // hex
#include <charconv>
#include <string_view>
#include <random>    // random_device

/**
ARGUMENTS
1: grid order
2: RNG seed (default: get from device)
*/
int main(const int argc, char const *const argv[]) {
	okiidoku::util::setup_console();

	// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays, cppcoreguidelines-pro-bounds-pointer-arithmetic)
	const unsigned user_order {(argc > 1)
		? static_cast<unsigned>(std::stoi(argv[1]))
		: okiidoku::compiled_orders[0]
	};
	const auto srand_key {[&]() -> std::uint_fast64_t {
		if (argc > 2) {
			const std::string_view arg {argv[2]};
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
	<< "\n- arg 1 (grid order) : " << user_order
	<< "\n- arg 2 (srand key)  : " << std::hex << srand_key << std::dec // TODO.mid ugh. it's using my numpunct grouping.
	<< std::endl;

	okiidoku::SharedRng shared_rng;
	shared_rng.rng.seed(srand_key);

	okiidoku::cli::Repl repl {user_order, shared_rng};
	repl.start();

	std::cout << "\nbye bye!\n" << std::endl;
	return 0;
}
