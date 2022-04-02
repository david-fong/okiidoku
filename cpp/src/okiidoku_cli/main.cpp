#include <okiidoku_cli/repl.hpp>
#include <okiidoku/rng.hpp>

#include <okiidoku_util/console_setup.hpp>

#include <string>
#include <iostream>  // cout,
#include <random>    // random_device,

/**
ARGUMENTS
1: grid order (default defined in okiidoku_config).
2: scramble random key (default: device random number).
*/
int main(const int argc, char const *const argv[]) {
	okiidoku::util::setup_console();

	unsigned int user_order; // 1
	std::uint_fast64_t srand_key; // 2

	user_order = (argc > 1) ? std::stoi(argv[1]) : M_OKIIDOKU_DEFAULT_ORDER;
	if (argc > 2 && !std::string(argv[2]).empty()) {
		srand_key = std::stoi(argv[2]);
	} else {
		srand_key = std::random_device()();
	}

	std::cout << "\nPARSED ARGUMENTS:"
	<< "\n- ARG 1 [[ grid order ]] : " << user_order
	<< "\n- ARG 2 [[ srand key  ]] : " << srand_key
	<< std::endl;

	// Scramble the random number generators:
	okiidoku::seed_rng(srand_key);

	okiidoku::cli::Repl repl {user_order};
	repl.start();

	// End of program:
	std::cout << "\nbye bye!\n" << std::endl;
	// std::cout << "\ntotal: " << okiidoku::gen::::total;
	// std::cout << "\ntrue_: " << okiidoku::gen::::true_ << std::endl;
	return 0;
}
