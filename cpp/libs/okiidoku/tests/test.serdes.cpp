// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <okiidoku/serdes.hpp>
#include <okiidoku/gen.hpp>
#include <okiidoku/grid.hpp>
#include <okiidoku/ints_io.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>  // fold_left
#include <functional> // plus
#include <array>
#include <span>
#include <memory>     // iwyu says I need this for `allocator`?
#include <cstdint>
#include <cstddef>    // size_t

namespace okiidoku::test {
template<Order O> OKIIDOKU_KEEP_FOR_DEBUG // NOLINTNEXTLINE(*-internal-linkage)
void test_serdes(const std::uint_fast32_t rng_seed) {
	INFO("testing for order ", O);
	CAPTURE(rng_seed);
	using namespace ::okiidoku::mono;
	OKIIDOKU_MONO_INT_TS_TYPEDEFS
	std::minstd_rand rng {rng_seed};

	std::array<Grid<O>, 8uz> grid_buf OKIIDOKU_DEFER_INIT; // NOLINT(*init*)
	std::array<std::size_t, 8uz> byte_counts {};
	const std::size_t num_grids {std::uniform_int_distribution{7uz, grid_buf.size()}(rng)};
	CAPTURE(num_grids);

	// TODO.high for some reason this fixes a bug... figure out what's going on pls
	// specifically, adding padding of `sizeof(buf_t)-1uz` null (`'\0'`) characters.
	// seems like write_solved appending to `os` has some issue? or maybe reading at the boundary between written grids...
	static constexpr auto arbitrary_padding {Int<1u,IntKind::constant>{}};

	const auto written_data {[&]{
		// serialize data to `std::string`:
		// { std::cin.get(); } // poor-person breakpoint. my bindings aren't working :(
		std::ostringstream os {std::ios::binary};
		REQUIRE_UNARY(os);
		for (auto i {0uz}; i < num_grids; ++i) { CAPTURE(i);
			auto& grid {grid_buf[i]};
			grid = generate_shuffled<O>(rng());
			REQUIRE_UNARY(os.good());
			byte_counts[i] += write_solved(grid, os);
			REQUIRE_UNARY_FALSE(os.fail());
			{
				for ([[maybe_unused]] const auto j : arbitrary_padding) { os.put('\0'); }
				byte_counts[i] += arbitrary_padding;
			}
			REQUIRE_EQ(
				static_cast<std::size_t>(os.tellp()),
				std::ranges::fold_left_first(std::span{byte_counts}.subspan(0uz,i+1uz), std::plus{})
				// std::accumulate(byte_counts.cbegin(), byte_counts.cbegin()+i+1uz, 0uz)
			);
		}
		return os.str();
	}()};
	REQUIRE_EQ(
		written_data.size(),
		std::ranges::fold_left_first(std::span{byte_counts}.subspan(0uz,num_grids), std::plus{})
		// std::accumulate(byte_counts.cbegin(), byte_counts.cbegin()+num_grids, 0uz)
	);
	std::istringstream is {written_data, std::ios::binary};
	REQUIRE_UNARY(is);
	// de-serialize data and check correctness:
	for (auto i {0uz}; i < num_grids; ++i) { CAPTURE(i);
		Grid<O> parsed_grid;
		std::size_t bytes_read {0uz};
		REQUIRE_UNARY(is.good());
		bytes_read += read_solved(parsed_grid, is);
		REQUIRE_UNARY_FALSE(is.fail());
		{
			is.ignore(arbitrary_padding);
			bytes_read += arbitrary_padding;
		}
		REQUIRE_EQ(byte_counts[i], bytes_read);
		REQUIRE_EQ(
			static_cast<std::size_t>(is.tellg()),
			std::ranges::fold_left_first(std::span{byte_counts}.subspan(0uz,i+1uz), std::plus{})
			// std::accumulate(byte_counts.cbegin(), byte_counts.cbegin()+i+1uz, 0uz)
		);
		REQUIRE_EQ(parsed_grid, grid_buf[i]);
	}
}}

TEST_CASE("okiidoku.serdes") {
	static constexpr std::uintmax_t num_rounds {1024u};
	// std::mt19937 rng {0u};
	std::mt19937 rng {std::random_device{}()};
	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
	for (std::uintmax_t round {0u}; round < num_rounds/((O_)*(O_)); ++round) { CAPTURE(round); \
		okiidoku::test::test_serdes<(O_)>(rng()); \
	}
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}