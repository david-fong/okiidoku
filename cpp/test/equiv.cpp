#include <solvent_lib/gen/mod.hpp>
#include <solvent_lib/equiv/canon.hpp>
#include <solvent_lib/equiv/scramble.hpp>
// #include <solvent_lib/print.hpp>
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
	unsigned int num_rounds; // 3

	user_order = (argc > 1) ? std::stoi(argv[1]) : SOLVENT_DEFAULT_ORDER;
	if (argc > 2 && !std::string(argv[2]).empty()) {
		srand_key = std::stoul(argv[2]);
	} else {
		srand_key = std::random_device()();
	}
	num_rounds = (argc > 3) ? std::stoi(argv[3]) : 1000;

	std::cout << "\nPARSED ARGUMENTS:"
	<< "\n- ARG 1 [[ grid order ]] : " << user_order
	<< "\n- ARG 2 [[ srand key  ]] : " << srand_key
	<< "\n- ARG 3 [[ num rounds ]] : " << num_rounds
	<< std::endl;

	// Scramble the random number generators:
	solvent::lib::gen::seed_rng(srand_key);
	solvent::lib::equiv::seed_scrambler_rng(srand_key);

	using namespace solvent::lib;
	unsigned int count_bad = 0;
	for (unsigned round = 0; round < num_rounds; round++) {
		auto gen = gen::Generator<3>();
		const auto result = gen(gen::Params {.canonicalize = true});
		if (result.status != gen::ExitStatus::Ok) { continue; }
		auto other_result = result; {
			auto const scrambled = equiv::scramble<3>(result.grid);
			other_result.grid = equiv::canonicalize<3>(scrambled);
		}
		if (result.grid != other_result.grid) {
			count_bad++;
			// TODO
			// result.print_serial(std::cout);
			// std::cout << "\n";
			// other_result.print_serial(std::cout);
			// std::cout << "\n";
		}
		std::cout << "\n";
	}
	std::cout << "\ncount bad: " << count_bad << " / " << num_rounds;
	return 0;
}
