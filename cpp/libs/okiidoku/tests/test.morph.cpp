#include <catch2/catch_test_macros.hpp>

#include <okiidoku/morph/canon.hpp>
#include <okiidoku/morph/scramble.hpp>
#include <okiidoku/gen.hpp>
#include <okiidoku/print_2d.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/o2_bit_arr.hpp>

#include <okiidoku_cli_utils/shared_rng.hpp>

#include <random> // random_device,

template<okiidoku::Order O>
void test_morph(okiidoku::SharedRng& shared_rng, const unsigned num_rounds) {
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


TEST_CASE("morph") {
	const auto default_num_rounds {100U};
	okiidoku::SharedRng shared_rng {};
	shared_rng.rng.seed(std::random_device()()); // look into using Catch2 GENERATE and random() features

	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
	test_morph<O_>(shared_rng, default_num_rounds);
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}