#include <okiidoku/fmt/print_2d.hpp>
// #include <okiidoku/visitor/gen/batch.hpp>
#include <okiidoku/visitor/gen/stochastic.hpp>
#include <okiidoku/visitor/morph/canon.hpp>
#include <okiidoku/visitor/morph/scramble.hpp>
#include <okiidoku/visitor/db/serdes.hpp>
#include <okiidoku/visitor/grid.hpp>
#include <okiidoku/shared_rng.hpp>

#include <okiidoku_cli_utils/console_setup.hpp>

#include <iostream>  // cout,
#include <string>
#include <random>    // random_device,
#include <array>

// OKIIDOKU_DEFINE_MT19937_64


// TODO.low experiment with effect of batching gen and then doing canon on that batch for perf
// TODO.high it should probably just return right away if it encounters any failure.
// returns the number of failures
template<okiidoku::Order O>
unsigned test_morph_O(const unsigned num_rounds) {
	using namespace okiidoku;
	using namespace okiidoku::mono;
	std::cout << "\n\ntesting for order " << O << std::endl;
	// Note: if gen_path gets un-deprecated, assert that paths are valid.

	unsigned int count_bad {0};
	for (unsigned round {0}; round < num_rounds; ) {
		gen::ss::Generator<O> g {};
		g();

		grid_arr_flat_t<O> gen_grid;
		g.write_to_(std::span(gen_grid));
		morph::canonicalize<O>(gen_grid);

		grid_arr_flat_t<O> canon_grid = gen_grid;
		morph::scramble<O>(canon_grid);
		morph::canonicalize<O>(canon_grid);

		if (gen_grid != canon_grid) {
			++count_bad;
			std::clog << "\n!bad\n";
			const std::array<print_2d_grid_view, 2> palette_ {
				[&](auto coord){ return gen_grid[coord]; },
				[&](auto coord){ return canon_grid[coord]; },
			};
			print_2d(std::clog, O, palette_);
			// std::clog << "\n";
			std::clog << "\n==========\n";
		} else {
			std::clog << ".";
		}
		++round;
	}
	std::clog.flush();
	std::cout << "\ncount bad: " << count_bad << " / " << num_rounds;
	if (count_bad > 0) {
		std::cerr << "\nerrors when testing order " << O;
	}
	return count_bad;
}


// TODO.high test serdes


/**
*/
int main(const int argc, char const *const argv[]) {
	okiidoku::util::setup_console();

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

	SharedRng shared_rng;
	shared_rng.rng.seed(srand_key);

	if (test_morph_O<3>(num_rounds)) {
		return 1;
	}
	if (test_morph_O<4>(num_rounds)) {
		// return 1;
	}
	if (test_morph_O<5>(num_rounds)) {
		// return 1;
	}
	if (test_morph_O<10>(num_rounds)) {
		// return 1;
	}

	// std::cout << "\ntotal: " << okiidoku::gen::ss::total;
	// std::cout << "\ntrue_: " << okiidoku::gen::ss::true_ << std::endl;
	return 0;
}