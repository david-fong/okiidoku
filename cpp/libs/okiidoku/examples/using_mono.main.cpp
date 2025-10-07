// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/morph/canon.hpp>
#include <okiidoku/morph/scramble.hpp>
#include <okiidoku/serdes.hpp>
#include <okiidoku/gen.hpp>
#include <okiidoku/print_2d.hpp>
#include <okiidoku/grid.hpp>

#include <okiidoku_cli_utils/shared_rng.hpp>
#include <okiidoku_cli_utils/console_setup.hpp>

#include <iostream>  // cout,
#include <random>    // random_device,
#include <array>
#include <cstdlib>   // EXIT_SUCCESS

/**
strong recommendation: do NOT do `using namespace okiidoku;` anywhere.
strong recommendation: do NOT use C++'s ADL mechanism to call non-operator
	functions defined in the okiidoku namespace in an unqualified manner.
you will probably not have a good time if you do any of those things.
*/

// "copy" names under `okiidoku::mono` to okiidoku:
// recommendation: don't do this in header files (only in .cpp files)
namespace okiidoku { using namespace ::okiidoku::mono; }

int main([[maybe_unused]] const int argc, [[maybe_unused]] char const *const argv[]) {
	namespace oki = ::okiidoku;
	oki::util::setup_console();
	oki::util::SharedRng shared_rng {std::random_device{}()};

	constexpr oki::Order O {3};

	oki::Grid<O> gen_grid;
	gen_grid.init_most_canonical();
	oki::shuffle(gen_grid, shared_rng());
	oki::Grid<O> puz_grid;
	// TODO.wait oki::make_minimal_puzzle(gen_grid, puz_of_gen_grid);
	auto canon_gen_grid_xform {oki::canonicalize(gen_grid)};
	canon_gen_grid_xform.apply_in_place(puz_grid);

	oki::Transformation<O> scramble_xform {};
	oki::scramble(scramble_xform, shared_rng());
	oki::Grid<O> canon_grid;
	scramble_xform.apply_from_to(gen_grid, canon_grid);
	oki::canonicalize(canon_grid);

	if (gen_grid != canon_grid) {
		// TODO.wait update this example comment when canonicalize works
		// this should never happen (once the library is correctly implemented)
		std::abort();
	}

	oki::print_2d(std::clog, shared_rng(), gen_grid, canon_grid);

	return EXIT_SUCCESS;
}