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

#include <sstream>
#include <random>
#include <cstdint>

namespace okiidoku::test {
template<Order O> OKIIDOKU_KEEP_FOR_DEBUG // NOLINTNEXTLINE(*-internal-linkage)
void test_serdes(
	const std::uintmax_t rng_seed,
	[[maybe_unused]] const std::uintmax_t round // for conditional breakpoints
) {
	CAPTURE(rng_seed);
	using namespace ::okiidoku::mono;
	OKIIDOKU_MONO_INT_TS_TYPEDEFS
	std::minstd_rand rng {static_cast<std::uint_fast32_t>(rng_seed)};

	const auto grid {generate_shuffled<O>(rng())}; CAPTURE(grid);

	// TODO test serdes of multiple grids to the same stream

	std::size_t byte_count {0uz};
	const auto written_data {[&,round]{
		(void)round; // for conditional breakpoint
		// serialize data to `std::string`:
		std::ostringstream os;
		byte_count += write_solved(grid, os);
		return os.str();
	}()};
	REQUIRE_EQ(written_data.size(), byte_count);
	{
		// de-serialize data and check correctness:
		Grid<O> parsed_grid;
		std::istringstream is {written_data};
		const auto bytes_read {read_solved(parsed_grid, is)};
		REQUIRE_EQ(bytes_read, byte_count);
		REQUIRE_EQ(parsed_grid, grid);
	}
}}

TEST_CASE("okiidoku.serdes") {
	static constexpr std::uintmax_t num_rounds {8192u};
	// std::mt19937_64 rng {0u};
	std::mt19937_64 rng {std::random_device{}()};
	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
	for (std::uintmax_t round {0u}; round < num_rounds/((O_)*(O_)); ++round) { CAPTURE(round); \
		okiidoku::test::test_serdes<(O_)>(rng(), round); \
	}
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}