// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <okiidoku/detail/mixed_radix_int_serdes.hpp>
#include <okiidoku_cli_utils/shared_rng.hpp>

#include <random>  // random_device,
#include <iostream>
#include <sstream>
#include <array>
#include <algorithm>
#include <limits>
#include <cstdint>
#include <cstdint>

namespace okiidoku { namespace {
OKIIDOKU_KEEP_FOR_DEBUG // NOLINTNEXTLINE(*-internal-linkage)
void test_mixed_radix_int_serdes(okiidoku::util::SharedRng& shared_rng, const std::uintmax_t num_rounds) {
	using namespace ::okiidoku::detail;
	using radix_t = std::uint_least8_t;
	using writer_t = MixedRadixUintWriter<std::uint_least8_t>;
	using reader_t = MixedRadixUintReader<std::uint_least8_t>;

	static constexpr std::size_t max_int_places {1024u};
	std::array<writer_t::Item, max_int_places> places_buf;

	for (std::uintmax_t round {0u}; round < num_rounds; ++round) {
		const size_t num_places {shared_rng() % max_int_places};
		std::size_t bytes_written {0uz};
		const auto written_data {[&,round]noexcept{
			// serialize data to `std::string`:
			writer_t writer;
			std::ostringstream os;
			for (auto i {0uz}; i < num_places; ++i) {
				auto& p {places_buf[i]};
				p.radix = std::max(radix_t{1u}, static_cast<radix_t>(shared_rng() % std::numeric_limits<radix_t>::max()));
				p.digit = static_cast<radix_t>(shared_rng() % p.radix);
				if (!writer.accept(p.radix, p.digit) || i+1uz == num_places) [[unlikely]] {
					writer.flush(os);
				}
				REQUIRE(writer.digits_written() == i+1uz);
			}
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
	}
}}}

TEST_CASE("okiidoku.mixed_radix_int_serdes") {
	okiidoku::util::SharedRng shared_rng {0u}; // TODO.high change this back to random device
	// okiidoku::util::SharedRng shared_rng {std::random_device{}()};
	okiidoku::test_mixed_radix_int_serdes(shared_rng, 1024u);
}