#include <okiidoku/puzzle/make.hpp>
#include <okiidoku/puzzle/solve.hpp>
#include <okiidoku/morph/canon.hpp>
#include <okiidoku/morph/scramble.hpp>
#include <okiidoku/serdes.hpp>
#include <okiidoku/print_2d.hpp>
#include <okiidoku/gen.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/o2_bit_arr.hpp>
#include <okiidoku/shared_rng.hpp>

#include <okiidoku_cli_utils/console_setup.hpp>

#include <iostream>  // cout,
#include <iomanip>   // hex
#include <charconv>
#include <string_view>
#include <random>    // random_device,
#include <array>

#ifdef NDEBUG
#define OKIIDOKU_NO_LOGGING
#undef NDEBUG
#endif
#include <cassert>


// TODO.high it should probably just return right away if it encounters any failure.
// returns the number of failures
template<okiidoku::Order O>
unsigned test_morph(okiidoku::SharedRng& shared_rng, const unsigned num_rounds) {
	using namespace okiidoku;
	using namespace okiidoku::mono;
	using T = Ints<O>;
	using o1x_t = int_ts::o1x_t<O>;
	using o2x_t = int_ts::o2x_t<O>;
	using o2i_t = int_ts::o2i_t<O>;
	using o3x_t = int_ts::o3x_t<O>;
	// using o4i_t = int_ts::o4i_t<O>;
	std::cout << "\n\ntesting for order " << O << std::endl;
	// Note: if gen_path gets un-deprecated, assert that paths are valid.

	assert(O2BitArr_ones<O>.count() == T::O2);
	for (o2i_t i {0}; i < T::O2; ++i) {
		assert(O2BitArr_ones<O>.count_set_bits_below(static_cast<o2x_t>(i)) == i);
	}
	{
		o2i_t count {0};
		for (auto walker {O2BitArr_ones<O>.set_bits_walker()}; walker.has_more(); walker.advance()) {
			assert(walker.value() == count);
			++count;
		}
		assert(count == T::O2);
	}

	for (o2i_t i {0}; i < T::O2; ++i) {
	for (o2i_t j {0}; j < T::O2; ++j) {
		const auto box_cell_rmi {box_cell_to_rmi<O>(i,j)};
		assert(rmi_to_box<O>(box_cell_rmi) == i);
		assert(rmi_to_box_cell<O>(box_cell_rmi) == j);

		const auto chute {static_cast<o1x_t>(i/T::O1)};
		const auto chute_cell {static_cast<o3x_t>(((i%T::O1)*T::O2)+j)};
		const auto h_chute_cell_rmi {chute_cell_to_rmi<O>(LineType::row, chute, chute_cell)};
		const auto v_chute_cell_rmi {chute_cell_to_rmi<O>(LineType::col, chute, chute_cell)};
		assert(h_chute_cell_rmi == ((i*T::O2)+j));
		assert(v_chute_cell_rmi == ((j*T::O2)+i));
	}}

	unsigned int count_bad {0};

	Grid<O> gen_grid;
	assert(grid_is_empty(gen_grid)); // grid by default-construction must be initialized to an empty grid

	// Grid<O> canon_grid;

	for (unsigned round {0}; round < num_rounds; ) {
		generate(gen_grid, shared_rng.get_u64());

		std::clog << "\nmaking puzzle #" << int(round);
		Grid<O> puz_grid {gen_grid};
		make_minimal_puzzle(puz_grid, shared_rng.get_u64());
		#ifndef OKIIDOKU_NO_LOGGING
		print_2d<O>(std::clog, shared_rng.get_u64(), gen_grid, puz_grid);
		#endif


		/* const auto gen_canon_transform {canonicalize(gen_grid)};
		if (gen_canon_transform.inverted().inverted() != gen_canon_transform) {
			std::clog << "\ntransformation twice-inverted must equal itself.";
			std::exit(1);
		}

		canon_grid = gen_grid;
		scramble(canon_grid, shared_rng.get_u64());
		canonicalize(canon_grid);

		if (gen_grid != canon_grid) {
			++count_bad;
			std::clog << "\n!bad\n";
			print_2d<O>(std::clog, shared_rng.get_u64(), gen_grid, canon_grid);
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

	if (test_morph<4>(shared_rng, num_rounds) != 0) { return 1; }
	// #define OKIIDOKU_FOR_COMPILED_O(O_) \
	// if (test_morph<O_>(shared_rng, num_rounds) != 0) { return 1; }
	// OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	// #undef OKIIDOKU_FOR_COMPILED_O

	return 0;
}