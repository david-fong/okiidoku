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

const unsigned DEFAULT_ORDER = 4;

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

	user_order = (argc > 1) ? std::stoi(argv[1]) : DEFAULT_ORDER;
	if (argc > 2 && !std::string(argv[2]).empty()) {
		srand_key = std::stoi(argv[2]);
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
	// (It will automatically enter its REPL).
	switch (static_cast<solvent::Order>(user_order)) {
		case 3: { solvent::cli::Repl<3> s3; s3.start(); break; }
		case 4: { solvent::cli::Repl<4> s4; s4.start(); break; }
		case 5: { solvent::cli::Repl<5> s5; s5.start(); break; }
		case 6: { solvent::cli::Repl<6> s6; s6.start(); break; }
		default: {
			std::cout << str::RED.ON << "\nILLEGAL ARGUMENT:\n  order must be one of: { ";
			for (unsigned i = 3; i <= 6; i++) {
				std::cout << i << ", ";
			}
			std::cout << "}" << str::RED.OFF << std::endl;
			break;
		}
	}

	// End of program:
	std::cout << "\nbye bye!\n" << std::endl;
	return 0;
}
