// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <catch2/catch_test_macros.hpp>

#include <okiidoku/puzzle/make.hpp>
#include <okiidoku/puzzle/solve.hpp>
#include <okiidoku/puzzle/ua_set.hpp>
#include <okiidoku/gen.hpp>
#include <okiidoku/print_2d.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/o2_bit_arr.hpp>

#include <okiidoku_cli_utils/shared_rng.hpp>

#include <random> // random_device,

template<okiidoku::Order O>
void test_puzzle(okiidoku::util::SharedRng& shared_rng, const unsigned num_rounds) {
	if (O >= 4) { return; } // TODO.high enable when solver for order=5 is faster?
	using namespace ::okiidoku;
	using namespace ::okiidoku::mono;
	// OKIIDOKU_MONO_INT_TS_TYPEDEFS
	INFO("testing for order " << O);

	Grid<O> gen_grid;
	init_most_canonical_grid(gen_grid);

	// Grid<O> canon_grid;

	for (unsigned round {0}; round < num_rounds; ++round) {
		generate_shuffled(gen_grid, shared_rng.get_rng_seed());
		REQUIRE(grid_follows_rule(gen_grid));

		INFO("making puzzle #" << int(round));
		Grid<O> puz_grid {gen_grid};
		auto ua_sets {find_size_4_minimal_unavoidable_sets(puz_grid)};
		// CAPTURE(unsigned(ua_sets.ua_set_4s.size());
		make_minimal_puzzle(puz_grid, shared_rng.get_rng_seed());
		// #ifndef OKIIDOKU_NO_LOGGING
		// print_2d<O>(std::clog, shared_rng.get_rng_seed(), gen_grid, puz_grid);
		// #endif
	}
}


TEST_CASE("puzzle") {
	const auto default_num_rounds {100U};
	okiidoku::util::SharedRng shared_rng {};
	shared_rng.rng.seed(std::random_device()()); // look into using Catch2 GENERATE and random() features

	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
	test_puzzle<O_>(shared_rng, default_num_rounds);
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}