#include "solvent/gen/backtracking.hpp"
#include "solvent/gen/stochastic.hpp"
#include "solvent/gen/batch.hpp"
#include "solvent/morph/canon.hpp"
#include "solvent/morph/scramble.hpp"
#include "solvent/print.hpp"
#include "solvent/grid.hpp"
#include "solvent/rng.hpp"

#include "solvent_util/console_setup.hpp"

#include <iostream>  // cout,
#include <string>
#include <random>    // random_device,
#include <array>


// TODO experiment with effect of batching gen and then doing canon on that batch for perf
// TODO it should probably just return right away if it encounters any failure.
// returns the number of failures
template<solvent::Order O>
unsigned test_morph_O(const unsigned num_rounds) {
	using namespace solvent;
	constexpr unsigned O4 {O*O*O*O};
	std::cout << "\n\ntesting for order " << O << std::endl;
	// TODO: assert that paths are valid.

	unsigned int count_bad {0};
	for (unsigned round {0}; round < num_rounds; ) {
		gen::ss::GeneratorO<O> g {};
		g(/* {} */);
		if (!g.status_is_ok()) {
			// std::array<typename solvent::size<O>::ord2i_t, O4> gen_grid;
			// g.write_to_(std::span(gen_grid));
			// std::clog << "bad: ";
			// print::text(std::clog, O, gen_grid);
			std::clog << "x"/*  << std::flush */;
			continue;
		}

		std::array<typename solvent::size<O>::ord2x_t, O4> gen_grid;
		g.write_to_(std::span(gen_grid));
		morph::canonicalize<O>(gen_grid);

		std::array<typename solvent::size<O>::ord2x_t, O4> canon_grid = gen_grid;
		morph::scramble<O>(canon_grid);
		morph::canonicalize<O>(canon_grid);

		if (gen_grid != canon_grid) {
			++count_bad;
			// TODO
			std::clog << "\n!bad\n";
			print::text(std::clog, O, gen_grid);
			std::clog << "\n";
			print::text(std::clog, O, canon_grid);
			std::clog << "\n==========\n";
		} else {
			std::clog << ".";
		}
		++round;
	}
	std::clog.flush();
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
	solvent::seed_rng(srand_key);

	// TODO change the test to try out all orders.
	if (test_morph_O<3>(num_rounds)) {
		return 1;
	}


	// using solvent::gen::batch::batch;
	// auto params = solvent::gen::batch::Params {
	// 	.gen_params{}, .only_count_oks=true, .stop_after=100
	// };
	// batch<3>(params, [](typename solvent::gen::Generator<3>::ResultView result){
	// 	std::cout << "\nhi";
	// 	result.to_generic().print_pretty(std::cout);
	// }).print(std::cout, 3);

	// playing with ranges
	/* using namespace solvent;
	std::cout << "\n";
	gen::GeneratorO<3> g {};
	g({});
	g.print_pretty(std::cout);
	std::cout << "\n"; */

	// std::cout << "\ntotal: " << solvent::gen::ss::total;
	// std::cout << "\ntrue_: " << solvent::gen::ss::true_ << std::endl;
	return 0;
}