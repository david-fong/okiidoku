#include <solvent_cli/repl.hpp>
#include <solvent_lib/gen/mod.hpp>
#include <solvent_lib/equiv/scramble.hpp>
#include <solvent_util/console_setup.hpp>

#include <string>
#include <iostream>  // cout,
#include <random>    // random_device,

/**
ARGUMENTS
1: grid order (default defined in solvent_config).
2: scramble random key (default: device random number).
*/
int main(const int argc, char const *const argv[]) {
	solvent::util::setup_console();

	unsigned int user_order; // 1
	unsigned int srand_key;  // 2

	user_order = (argc > 1) ? std::stoi(argv[1]) : SOLVENT_DEFAULT_ORDER;
	if (argc > 2 && !std::string(argv[2]).empty()) {
		srand_key = std::stoul(argv[2]);
	} else {
		srand_key = std::random_device()();
	}

	std::cout << "\nPARSED ARGUMENTS:"
	<< "\n- ARG 1 [[ grid order ]] : " << user_order
	<< "\n- ARG 2 [[ srand key  ]] : " << srand_key
	<< std::endl;

	// Scramble the random number generators:
	solvent::lib::gen::seed_rng(srand_key);
	solvent::lib::equiv::seed_scrambler_rng(srand_key);

	solvent::cli::Repl repl { user_order };
	repl.start();

	// End of program:
	std::cout << "\nbye bye!\n" << std::endl;
	return 0;
}
