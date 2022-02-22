#include <solvent_lib/gen/mod.hpp>
#include <solvent_lib/morph/canon.hpp>
#include <solvent_lib/morph/scramble.hpp>
// #include <solvent_lib/print.hpp>
#include <solvent_util/console_setup.hpp>
#include <solvent_lib/grid.hpp>

#include <solvent_lib/morph/rel_prob.hpp>

#include <string>
#include <iostream>  // cout,
#include <random>    // random_device,

// TODO change the test to try out all orders.
constexpr uint8_t O = 3;

// TODO experiment with effect of batching gen and then doing canon on that batch for perf

/**
*/
int main(const int argc, char const *const argv[]) {
	solvent::util::setup_console();

	unsigned int user_order; // 1
	unsigned int srand_key;  // 2
	unsigned int num_rounds; // 3

	user_order = (argc > 1) ? std::stoi(argv[1]) : O;
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


	using namespace solvent::lib;

	// Scramble the random number generators:
	solvent::lib::gen::seed_rng(srand_key);
	solvent::lib::morph::seed_scrambler_rng(srand_key);

	unsigned int count_bad = 0;
	for (unsigned round = 0; round < num_rounds; ) {
		auto gen = gen::Generator<O>();
		const auto result = gen(gen::Params {.canonicalize = true});
		if (result.status != gen::ExitStatus::Ok) { continue; }
		round++;
		auto other_result = result; {
			other_result.grid = morph::canonicalize<O>(morph::scramble<O>(result.grid));
		}
		if (result.grid != other_result.grid) {
			count_bad++;
			// TODO
			std::cout << "\n!bad\n";
			result.print_serial(std::cout);
			std::cout << "\n";
			other_result.print_serial(std::cout);
			std::cout << "\n";
		}
		std::cout << "\n";
	}
	std::cout << "\ncount bad: " << count_bad << " / " << num_rounds;
	return 0;
}