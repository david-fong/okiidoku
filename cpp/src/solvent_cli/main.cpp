#include <solvent_cli/repl.hpp>
#include <solvent_util/str.hpp>

#if WINDOWS_ANSI
#include <windows.h>
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#include <string>
#include <iostream>  // cout,
#include <random>    // random_device,

namespace str = solvent::util::str;

/**
 * ARGUMENTS
 * 0. executable name (fixed).
 * 1. grid order (default: 4).
 * 2. scramble random key (default: time).
 */
int main(const int argc, char const *const argv[]) {
	#if WINDOWS_ANSI
	DWORD mode;
	GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &mode);
	mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), mode);
	#endif

	// My implementation specifies this as safe:
	std::ios_base::sync_with_stdio(false);

	unsigned int user_order; // 1
	unsigned int srand_key;  // 2

	user_order = (argc > 1) ? std::stoi(argv[1]) : SOLVENT_DEFAULT_ORDER;
	if (argc > 2 && !std::string(argv[2]).empty()) {
		srand_key = std::stoul(argv[2]);
	} else {
		srand_key = std::random_device()();
	}

	std::cout << "\nPARSED ARGUMENTS:"
	<< "\n- ARG 1 [[ grid order  ]] : " << user_order
	<< "\n- ARG 2 [[ srand key   ]] : " << srand_key
	<< std::endl;

	// Scramble the random number generator (std::rand is no longer used):
	solvent::lib::gen::Rng.seed(srand_key);

	// Create a Solver of the specified order:
	solvent::cli::Repl repl { user_order };
	repl.start();

	// End of program:
	std::cout << "\nbye bye!\n" << std::endl;
	return 0;
}
