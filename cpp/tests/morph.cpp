#include <solvent_lib/gen/mod.hpp>
#include <solvent_lib/morph/canon.hpp>
#include <solvent_lib/morph/scramble.hpp>
// #include <solvent_lib/print.hpp>
#include <solvent_util/console_setup.hpp>
#include <solvent_lib/grid.hpp>

#include <string>
#include <iostream>  // cout,
#include <random>    // random_device,


// TODO experiment with effect of batching gen and then doing canon on that batch for perf
// TODO it should probably just return right away if it encounters any failure.
// returns the number of failures
template<solvent::Order O>
unsigned test_morph_O(const unsigned num_rounds) {
	using namespace solvent::lib;
	std::cout << "\n\ntesting morph for order " << O << '\n';
	unsigned int count_bad = 0;
	for (unsigned round = 0; round < num_rounds; ) {
		auto gen = gen::Generator<O>();
		const auto result = gen(gen::Params {.canonicalize = true});
		if (result.status != gen::ExitStatus::Ok) { continue; }
		round++;
		auto other_result = result; {
			const auto scrambled = morph::scramble<O>(result.grid);
			other_result.grid = morph::canonicalize<O>((std::span{scrambled}).template subspan<0,O*O*O*O>()); // yuck
		}
		if (result.grid != other_result.grid) {
			count_bad++;
			// TODO
			std::cout << "\n!bad\n";
			result.print_text(std::cout);
			std::cout << "\n";
			other_result.print_text(std::cout);
			std::cout << "\n==========\n";
		} else {
			std::cout << ".";
		}
	}
	std::cout << "\ncount bad: " << count_bad << " / " << num_rounds;
	return count_bad;
}


/**
*/
int main(const int argc, char const *const argv[]) {
	solvent::util::setup_console();

	std::uint_fast64_t srand_key;  // 1
	unsigned int num_rounds; // 2

	if (argc > 1 && !std::string(argv[1]).empty()) {
		srand_key = std::stoi(argv[1]);
	} else {
		srand_key = std::random_device()();
	}
	num_rounds = (argc > 2) ? std::stoi(argv[2]) : 1000;

	std::cout << "\nPARSED ARGUMENTS:"
	<< "\n- ARG 1 [[ srand key  ]] : " << srand_key
	<< "\n- ARG 2 [[ num rounds ]] : " << num_rounds
	<< std::endl;

	// Scramble the random number generators:
	solvent::lib::gen::seed_rng(srand_key);
	solvent::lib::morph::seed_scrambler_rng(srand_key);

	// TODO change the test to try out all orders.
	if (test_morph_O<3>(num_rounds)) {
		return 1;
	}
	return 0;
}