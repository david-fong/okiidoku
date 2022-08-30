// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku_cli_utils/shared_rng.hpp>
#include <okiidoku_cli_utils/console_setup.hpp>
#include <okiidoku/gen.hpp>

#include <iostream>  // cout
#include <iomanip>   // hex
#include <charconv>
#include <string_view>
#include <random>    // random_device

template<okiidoku::Order O>
void do_training(okiidoku::util::SharedRng& shared_rng, const unsigned num_rounds) {
	// 3: 30k, 4: 40k, 5: 700,
	using namespace ::okiidoku;
	using namespace ::okiidoku::mono;
	// OKIIDOKU_MONO_INT_TS_TYPEDEFS

	Grid<O> gen_grid;
	init_most_canonical_grid(gen_grid);

	// Grid<O> canon_grid;

	for (unsigned round {0}; round < num_rounds; ++round) {
		generate_shuffled(gen_grid, shared_rng.get_rng_seed());

		/* const auto gen_canon_transform {canonicalize(gen_grid)};
		canon_grid = gen_grid;
		scramble(canon_grid, shared_rng.get_rng_seed());
		canonicalize(canon_grid);
		*/
	}
}


/**
ARGUMENTS
1: RNG seed (default: get from device)
*/
int main(const int argc, char const *const argv[]) {
	okiidoku::util::setup_console();

	// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays, cppcoreguidelines-pro-bounds-pointer-arithmetic)
	const auto srand_key {[&]() -> std::uint_fast64_t {
		if (argc > 1) {
			const std::string_view arg {argv[1]};
			std::uint_fast64_t parsed {};
			if (std::from_chars(arg.data(), arg.data()+arg.size(), parsed, 16).ec == std::errc{}) {
				return parsed;
			}
			std::cerr << "\nfailed to parse rng seed argument (hex u64). using random_device instead.";
		}
		return std::random_device()();
	}()};
	// NOLINTEND(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays, cppcoreguidelines-pro-bounds-pointer-arithmetic)

	std::cout << "\nparsed arguments:"
	<< "\n- arg 1 (srand key)  : " << std::hex << srand_key << std::dec // TODO.mid ugh. it's using my numpunct grouping.
	<< std::endl;

	okiidoku::util::SharedRng shared_rng(srand_key);
	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
	do_training<O_>(shared_rng, 1000);
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
	return 0;
}
