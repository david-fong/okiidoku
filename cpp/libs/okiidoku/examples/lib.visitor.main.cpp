#include <okiidoku/morph/canon.hpp>
#include <okiidoku/morph/scramble.hpp>
#include <okiidoku/db/serdes.hpp>
#include <okiidoku/gen.hpp>
#include <okiidoku/fmt/print_2d.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/shared_rng.hpp>

#include <okiidoku_cli_utils/console_setup.hpp>

#include <iostream>  // cout,
#include <random>    // random_device,
#include <array>

// TODO.asap update this to match mono once the mono example is more refined.

// "copy" names under `okiidoku::visitor` to okiidoku:
namespace okiidoku {
	using namespace okiidoku::visitor;
}

int main([[maybe_unused]] const int argc, [[maybe_unused]] char const *const argv[]) {
	namespace oki = okiidoku;
	oki::util::setup_console();
	oki::SharedRng shared_rng;
	shared_rng.rng.seed(std::random_device()());

	constexpr oki::Order O = 3;

	oki::Grid gen_grid(O);
	// oki::generate(gen_grid, shared_rng);
	// oki::morph::canonicalize(gen_grid);

	oki::Grid canon_grid {gen_grid};
	// oki::morph::scramble(canon_grid, shared_rng);
	// oki::morph::canonicalize(canon_grid);

	{
		const auto palette_ {std::to_array<oki::print_2d_grid_view>({
			[&](auto coord){ return   gen_grid.at_row_major(coord); },
			[&](auto coord){ return canon_grid.at_row_major(coord); },
		})};
		print_2d(std::clog, O, palette_, shared_rng);
	}

	return 0;
}