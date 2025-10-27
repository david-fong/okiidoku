// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/morph/canon.hpp>
#include <okiidoku/morph/scramble.hpp>
#include "okiidoku/morph/transform.hpp"
#include <catch2/catch_test_macros.hpp>

#include <okiidoku/gen.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/ints_io.hpp>
#include <okiidoku/order.hpp>
#include <okiidoku/detail/util.hpp>

#include <okiidoku_cli_utils/shared_rng.hpp>

#include <random>  // random_device,
#include <cstdint>

namespace okiidoku::test {
template<Order O> OKIIDOKU_KEEP_FOR_DEBUG // NOLINTNEXTLINE(*-internal-linkage)
void test_morph(okiidoku::util::SharedRng& shared_rng, const std::uintmax_t num_rounds) {
	INFO("testing for order " << unsigned{O});
	if constexpr (O >= 4) { return; } // TODO.mid enable when solver for order=5 is faster?
	using namespace ::okiidoku::mono;
	// OKIIDOKU_MONO_INT_TS_TYPEDEFS

	Grid<O> gen_grid;
	gen_grid.init_most_canonical();

	Grid<O> canon_grid;
	Transformation<O> scramble_xform {};
	CHECK(scramble_xform.inverted().inverted() == scramble_xform);

	for (std::uintmax_t round {0u}; round < num_rounds; ++round) { CAPTURE(round);
		shuffle(gen_grid, shared_rng());
		CHECK(gen_grid.follows_rule());

		const auto gen_canon_xform {canonicalize(gen_grid)};
		CHECK(gen_canon_xform.inverted().inverted() == gen_canon_xform);
		// TODO: test that performing various transforms and then performing the invert truly reverts the transform.

		scramble(scramble_xform, shared_rng());
		CHECK(scramble_xform.inverted().inverted() == scramble_xform);
		scramble_xform.apply_from_to(gen_grid, canon_grid);

		const auto scramble_canon_xform {canonicalize(canon_grid)};
		CHECK(scramble_canon_xform.inverted().inverted() == scramble_canon_xform);
		CHECK(scramble_canon_xform.inverted() == scramble_xform);
		CHECK(scramble_xform.inverted() == scramble_canon_xform);

		CHECK(gen_grid == canon_grid);
	}
}}

TEST_CASE("okiidoku.morph") {
	constexpr auto default_num_rounds {100u};
	okiidoku::util::SharedRng shared_rng {std::random_device{}()};

	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
	okiidoku::test::test_morph<(O_)>(shared_rng, default_num_rounds);
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}