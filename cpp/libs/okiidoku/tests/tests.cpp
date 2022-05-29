#include <okiidoku/puzzle/make.hpp>
#include <okiidoku/puzzle/solve.hpp>
#include <okiidoku/morph/canon.hpp>
#include <okiidoku/morph/scramble.hpp>
#include <okiidoku/serdes.hpp>
#include <okiidoku/print_2d.hpp>
#include <okiidoku/gen.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/house_mask.hpp>
#include <okiidoku/shared_rng.hpp>

#include <okiidoku_cli_utils/console_setup.hpp>

#include <iostream>  // cout,
#include <iomanip>   // hex
#include <charconv>
#include <string_view>
#include <random>    // random_device,
#include <array>
#include <cassert>

// OKIIDOKU_DEFINE_MT19937_64
#ifdef NDEBUG
#undef NDEBUG
#endif


// TODO.high it should probably just return right away if it encounters any failure.
// returns the number of failures
template<okiidoku::Order O>
unsigned test_morph(okiidoku::SharedRng& shared_rng, const unsigned num_rounds) {
	using namespace okiidoku;
	using namespace okiidoku::mono;
	using T = Ints<O>;
	using o2i_t = int_ts::o2i_t<O>;
	// using o4i_t = int_ts::o4i_t<O>;
	std::cout << "\n\ntesting for order " << O << std::endl;
	// Note: if gen_path gets un-deprecated, assert that paths are valid.

	assert(house_mask_ones<O>.count() == T::O2);

	for (o2i_t i {0}; i < T::O2; ++i) {
	for (o2i_t j {0}; j < T::O2; ++j) {
		const auto rmi {box_cell_to_rmi<O>(i,j)};
		assert(rmi_to_box<O>(rmi) == i);
	}}

	unsigned int count_bad {0};

	Grid<O> gen_grid;
	if (!std::all_of(
		gen_grid.get_underlying_array().cbegin(),
		gen_grid.get_underlying_array().cend(),
		[](const auto val){ return val == T::O2; }
	)) {
		std::clog << "\ngrid must be initialized to an empty grid.";
		std::exit(1);
	}

	Grid<O> canon_grid;

	for (unsigned round {0}; round < num_rounds; ) {
		generate(gen_grid, shared_rng);

		std::clog << "\nmaking puzzle";
		Grid<O> puz_grid {gen_grid};
		make_minimal_puzzle(puz_grid); {
			const auto palette_ {std::to_array<print_2d_grid_view>({
				[&](auto rmi){ return gen_grid.at_rmi(rmi); },
				[&](auto rmi){ return puz_grid.at_rmi(rmi); },
			})};
			print_2d(std::clog, O, palette_, shared_rng);
		}

		/* const auto gen_canon_transform {canonicalize(gen_grid)};
		if (gen_canon_transform.inverted().inverted() != gen_canon_transform) {
			std::clog << "\ntransformation twice-inverted must equal itself.";
			std::exit(1);
		}

		canon_grid = gen_grid;
		scramble(canon_grid, shared_rng);
		canonicalize(canon_grid);

		if (gen_grid != canon_grid) {
			++count_bad;
			std::clog << "\n!bad\n";
			const auto palette_ {std::to_array<print_2d_grid_view>({
				[&](auto rmi){ return   gen_grid.at_rmi(rmi); },
				[&](auto rmi){ return canon_grid.at_rmi(rmi); },
			})};
			print_2d(std::clog, O, palette_, shared_rng);
			// std::clog << "\n";
			std::clog << "\n==========\n";
		} else {
			std::clog << ".";
		} */
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

	// TODO.mid design a way to extract this duplicate code (see cli.main.cpp) to utilities
	// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic,*-avoid-c-arrays)
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
	const unsigned int num_rounds {(argc > 2) ? static_cast<unsigned>(std::stoi(argv[2])) : 1000U};
	// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic,*-avoid-c-arrays)

	std::cout << "\nparsed arguments:"
	<< "\n- arg 1 (srand key)  : " << std::hex << srand_key << std::dec
	<< "\n- arg 2 (num rounds) : " << num_rounds
	<< std::endl;

	okiidoku::SharedRng shared_rng;
	shared_rng.rng.seed(srand_key);

	#define OKIIDOKU_FOR_COMPILED_O(O_) \
	if (test_morph<O_>(shared_rng, num_rounds) != 0) { return 1; }
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O

	return 0;
}