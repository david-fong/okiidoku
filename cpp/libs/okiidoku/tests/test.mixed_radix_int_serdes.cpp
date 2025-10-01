// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <okiidoku/detail/mixed_radix_int_serdes.hpp>

#include <random>  // random_device,
#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>
#include <array>
#include <limits>
#include <cstdint>

namespace okiidoku { namespace {
template<class T> using uidist_t = std::uniform_int_distribution<T>;

OKIIDOKU_KEEP_FOR_DEBUG // NOLINTNEXTLINE(*-internal-linkage)
void test_mixed_radix_int_serdes(
	const std::uintmax_t rng_seed,
	[[maybe_unused]] const std::uintmax_t round // for conditional breakpoints
) {
	using radix_t = std::uint_least8_t;
	using writer_t = detail::MixedRadixUintWriter<radix_t>; // TODO test with BufType other than default (currently failing)
	using reader_t = detail::MixedRadixUintReader<radix_t>;

	std::minstd_rand rng {static_cast<std::uint_fast32_t>(rng_seed)};

	// TODO write and read multiple ints to the same stringstream per round.

	std::array<writer_t::Item, 1024uz> places_buf;
	const size_t num_places {[&]{
		uidist_t<std::uintmax_t> num_places_dist {0u, places_buf.size()-1u};
		return num_places_dist(rng);
	}()}; CAPTURE(num_places);

	std::size_t bytes_written {0uz};
	const auto written_data {[&,round]noexcept{
		// serialize data to `std::string`:
		writer_t writer;
		std::ostringstream os;
		uidist_t<radix_t> radix_dist {1u};
		for (auto i {0uz}; i < num_places; ++i) { CAPTURE(i);
			auto& p {places_buf[i]};
			p.radix = radix_dist(rng);
			p.digit = uidist_t<radix_t>{0u,static_cast<radix_t>(p.radix-1u)}(rng);
			if (!writer.accept(p.radix, p.digit)) [[unlikely]] {
				REQUIRE(writer.digits_written() == i+1uz);
				writer.flush(os);
		}	}
		writer.flush(os);
		bytes_written = writer.bytes_written();
		return os.str();
	}()};
	{
		// de-serialize data and check correctness:
		std::istringstream is {written_data};
		reader_t reader;
		for (auto i {0uz}; i < num_places; ++i) {
			const auto& p {places_buf[i]};
			const auto digit {reader.read(is, p.radix)};
			REQUIRE(reader.digits_read() == i+1uz);
			REQUIRE(digit == p.digit);
		}
		reader.finish(is);
		REQUIRE(reader.bytes_read() == bytes_written);
	}
}}}

TEST_CASE("okiidoku.mixed_radix_int_serdes") {
	static constexpr std::uintmax_t num_rounds {1024u};
	std::mt19937_64 rng {std::random_device{}()};
	for (std::uintmax_t round {0u}; round < num_rounds; ++round) {
		CAPTURE(round);
		okiidoku::test_mixed_radix_int_serdes(rng(), round);
	}
}