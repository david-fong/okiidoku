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
	using o2x_t = int_ts::o2x_t<O>;
	using o2i_t = int_ts::o2i_t<O>;
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

	for (o2i_t box {0}; box < T::O2; ++box) {
	for (o2i_t box_cell {0}; box_cell < T::O2; ++box_cell) {
		const auto rmi {box_cell_to_rmi<O>(box,box_cell)};
		assert(rmi_to_box<O>(rmi) == box);
		assert(rmi_to_box_cell<O>(rmi) == box_cell);
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
		print_2d<O>(std::clog, shared_rng.get_u64(), gen_grid, puz_grid);


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

	#define OKIIDOKU_FOR_COMPILED_O(O_) \
	if (test_morph<O_>(shared_rng, num_rounds) != 0) { return 1; }
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O

	return 0;
}