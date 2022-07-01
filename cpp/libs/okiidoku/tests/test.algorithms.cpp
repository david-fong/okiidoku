#include <catch2/catch_test_macros.hpp>

#include <okiidoku/puzzle/make.hpp>
#include <okiidoku/puzzle/solve.hpp>
#include <okiidoku/puzzle/ua_set.hpp>
#include <okiidoku/morph/canon.hpp>
#include <okiidoku/morph/scramble.hpp>
#include <okiidoku/serdes.hpp>
#include <okiidoku/gen.hpp>
#include <okiidoku/print_2d.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/o2_bit_arr.hpp>

#include <okiidoku_cli_utils/shared_rng.hpp>
// #include <okiidoku_cli_utils/console_setup.hpp>

#include <random> // random_device,

template<okiidoku::Order O>
void test_algorithms(okiidoku::SharedRng& shared_rng, const unsigned num_rounds) {
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


		/* const auto gen_canon_transform {canonicalize(gen_grid)};
		if (gen_canon_transform.inverted().inverted() != gen_canon_transform) {
			std::clog << "\ntransformation twice-inverted must equal itself.";
			std::exit(1);
		}

		canon_grid = gen_grid;
		scramble(canon_grid, shared_rng.get_rng_seed());
		canonicalize(canon_grid);

		REQUIRE(gen_grid == canon_grid);
		*/
	}
}


TEST_CASE("algorithms") {
	const auto default_num_rounds {100U};
	okiidoku::SharedRng shared_rng {};
	shared_rng.rng.seed(std::random_device()()); // look into using Catch2 GENERATE and random() features

	#define OKIIDOKU_FOR_COMPILED_O(O_) \
	test_algorithms<O_>(shared_rng, default_num_rounds);
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}