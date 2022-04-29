#include <okiidoku/morph/canon.hpp>
#include <okiidoku/morph/scramble.hpp>
#include <okiidoku/db/serdes.hpp>
#include <okiidoku/fmt/print_2d.hpp>
#include <okiidoku/gen.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/shared_rng.hpp>

#include <okiidoku_cli_utils/console_setup.hpp>

#include <iostream>  // cout,
#include <string_view>
#include <random>    // random_device,
#include <array>

// OKIIDOKU_DEFINE_MT19937_64


// TODO.high it should probably just return right away if it encounters any failure.
// returns the number of failures
template<okiidoku::Order O>
unsigned test_morph(okiidoku::SharedRng& shared_rng, const unsigned num_rounds) {
	using namespace okiidoku;
	using namespace okiidoku::mono;
	// using T = traits<O>;
	std::cout << "\n\ntesting for order " << O << std::endl;
	// Note: if gen_path gets un-deprecated, assert that paths are valid.

	unsigned int count_bad {0};
	for (unsigned round {0}; round < num_rounds; ) {
		GridArr<O> gen_grid;
		generate<O>(gen_grid, shared_rng);
		morph::canonicalize<O>(gen_grid);

		GridArr<O> canon_grid {gen_grid};
		morph::scramble<O>(canon_grid, shared_rng);
		morph::canonicalize<O>(canon_grid);

		if (gen_grid != canon_grid) {
			++count_bad;
			std::clog << "\n!bad\n";
			const std::array<print_2d_grid_view, 2> palette_ {
				[&](auto coord){ return   gen_grid.at_row_major(coord); },
				[&](auto coord){ return canon_grid.at_row_major(coord); },
			};
			print_2d(std::clog, O, palette_, shared_rng);
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

	if (argc > 1 && !std::string_view(argv[1]).empty()) {
		srand_key = std::stoi(argv[1]);
	} else {
		srand_key = std::random_device()();
	}
	num_rounds = (argc > 2) ? std::stoi(argv[2]) : 1000;

	std::cout << "\nPARSED ARGUMENTS:"
	<< "\n- ARG 1 [[ srand key  ]] : " << srand_key
	<< "\n- ARG 2 [[ num rounds ]] : " << num_rounds
	<< std::endl;

	okiidoku::SharedRng shared_rng;
	shared_rng.rng.seed(srand_key);

	if (test_morph<3>(shared_rng, num_rounds)) {
		return 1;
	}
	if (test_morph<4>(shared_rng, num_rounds)) {
		// return 1;
	}
	if (test_morph<5>(shared_rng, num_rounds)) {
		// return 1;
	}
	if (test_morph<10>(shared_rng, num_rounds)) {
		// return 1;
	}

	return 0;
}