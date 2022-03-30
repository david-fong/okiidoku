// #include <ookiidoku/gen/batch.hpp>
#include <ookiidoku/gen/stochastic.hpp>
#include <ookiidoku/morph/canon.hpp>
#include <ookiidoku/morph/scramble.hpp>
#include <ookiidoku/print_2d.hpp>
#include <ookiidoku/db/serdes.hpp>
#include <ookiidoku/grid.hpp>
#include <ookiidoku/rng.hpp>

#include <ookiidoku_util/console_setup.hpp>

#include <iostream>  // cout,
#include <string>
#include <random>    // random_device,
#include <array>


// TODO.low experiment with effect of batching gen and then doing canon on that batch for perf
// TODO.high it should probably just return right away if it encounters any failure.
// returns the number of failures
template<ookiidoku::Order O>
unsigned test_morph_O(const unsigned num_rounds) {
	using namespace ookiidoku;
	constexpr unsigned O4 {O*O*O*O};
	std::cout << "\n\ntesting for order " << O << std::endl;
	// Note: if gen_path gets un-deprecated, assert that paths are valid.

	unsigned int count_bad {0};
	for (unsigned round {0}; round < num_rounds; ) {
		gen::ss::GeneratorO<O> g {};
		g();

		std::array<typename ookiidoku::size<O>::ord2x_t, O4> gen_grid;
		g.write_to_(std::span(gen_grid));
		morph::canonicalize<O>(gen_grid);

		std::array<typename ookiidoku::size<O>::ord2x_t, O4> canon_grid = gen_grid;
		morph::scramble<O>(canon_grid);
		morph::canonicalize<O>(canon_grid);

		if (gen_grid != canon_grid) {
			++count_bad;
			std::clog << "\n!bad\n";
			const std::array<print_2d_palette, 2> palette_ {
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
	ookiidoku::util::setup_console();

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
	ookiidoku::seed_rng(srand_key);

	if (test_morph_O<3>(num_rounds)) {
		// return 1;
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

	// std::cout << "\ntotal: " << ookiidoku::gen::ss::total;
	// std::cout << "\ntrue_: " << ookiidoku::gen::ss::true_ << std::endl;
	return 0;
}