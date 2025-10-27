// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <iostream>
#include <okiidoku/detail/mixed_radix_uint_serdes.hpp>
#include <catch2/catch_test_macros.hpp>

#include <okiidoku/detail/util.hpp>

// import std;
#include <sstream>
#include <random>
#include <string>
#include <array>
#include <cstdint>
#include <climits>   // CHAR_BIT
#include <cstddef>   // size_t
#include <concepts>  // unsigned_integral

namespace okiidoku::test {
	namespace { template<class T> using uidist_t = std::uniform_int_distribution<T>; }

	template <detail::Radix radix_t, std::unsigned_integral buf_t = std::uintmax_t>
	OKIIDOKU_KEEP_FOR_DEBUG // NOLINTNEXTLINE(*-internal-linkage)
	void test_mixed_radix_uint_serdes_one_round(
		const std::uint_fast32_t rng_seed
	) {
		INFO("sizeof(radix_t) := " << sizeof(radix_t));
		INFO("sizeof(buf_t)   := " << sizeof(buf_t));
		CAPTURE(rng_seed);
		using Writer = detail::MixedRadixUintWriter<radix_t,buf_t>;
		using Reader = detail::MixedRadixUintReader<radix_t,buf_t>;
		std::minstd_rand rng {rng_seed};

		// TODO write and read multiple ints to the same stringstream per round.

		static constexpr std::size_t max_num_places {1024u};
		std::array<typename Writer::Item, max_num_places> places_buf OKIIDOKU_DEFER_INIT; // NOLINT(*-init)
		const std::size_t num_places {uidist_t<std::size_t>{0u, max_num_places}(rng)}; CAPTURE(num_places);

		std::size_t byte_count {0uz}; CAPTURE(byte_count);
		const auto written_data {[&]{
			// serialize data to `std::string`:
			Writer writer;
			std::ostringstream os {std::ios::binary};
			REQUIRE(os.good());
			std::geometric_distribution<radix_t> radix_dist {1.0/(CHAR_BIT*sizeof(buf_t))};
			for (auto place {0uz}; place < num_places; ++place) { CAPTURE(place);
				auto& p {places_buf[place]};
					p.radix = 0u; do { p.radix = radix_dist(rng); } while (p.radix == 0u);
					p.digit = uidist_t<radix_t>{0u,static_cast<radix_t>(p.radix-1u)}(rng);
				if (!writer.accept(p)) [[unlikely]] {
					REQUIRE(os.good());
					writer.flush(os);
					REQUIRE_FALSE(os.fail());
				}
				REQUIRE(writer.item_count() == place+1uz);
			}
			REQUIRE(os.good());
			writer.flush(os);
			REQUIRE_FALSE(os.fail());
			byte_count += writer.byte_count();
			REQUIRE(static_cast<std::size_t>(os.tellp()) == byte_count);
			return os.str();
		}()};
		REQUIRE(written_data.size() == byte_count);
		{
			// de-serialize data and check correctness:
			std::istringstream is {written_data, std::ios::binary}; // TODO.high do I need to add ios::in here? is ::binary even needed? I'm not even passing it to create the ostringstream
			REQUIRE(is);
			Reader reader {byte_count};
			for (auto place {0uz}; place < num_places; ++place) { CAPTURE(place);
				const auto& expected {places_buf[place]}; CAPTURE(expected.radix);
				REQUIRE(is.good());
				const auto digit {reader.read(is, expected.radix)};
				REQUIRE_FALSE(is.fail());
				/* ^we don't expect to hit EOF here, but with more complex data lifetime,
				/ transfer, that's a real possibility. Ex. with a somehow truncated data
				archive file, or a network error. */
				REQUIRE(reader.item_count() == place+1uz);
				REQUIRE(digit == expected.digit);
			}
			REQUIRE(reader.byte_count() == 0uz);
			REQUIRE_FALSE(is.bad()); // non-recoverable error
			CHECK_FALSE(is.fail()); // error
			// CHECK(is.eof());
			const auto is_pos {static_cast<std::size_t>(is.tellg())};
			REQUIRE(is_pos == byte_count);
		}
	}


	// template <detail::Radix radix_t, std::unsigned_integral buf_t = std::uintmax_t>
	// OKIIDOKU_KEEP_FOR_DEBUG // NOLINTNEXTLINE(*-internal-linkage)
	// void test_mixed_radix_uint_serdes(
	// 	const std::uintmax_t rng_seed
	// ) {
	// 	// TODO for(...) { test_mixed_radix_uint_serdes_one_round(); }
	// }
}


TEST_CASE("okiidoku.uint_serdes") {
	const std::uint64_t i {0xFEDC'BA98'7654'3210uLL};
	std::ostringstream os {std::ios::binary};
	os.write(reinterpret_cast<const char*>(&i), sizeof(i)); // NOLINT(*-cast)
	REQUIRE_FALSE(os.fail());
	const auto str {os.str()};
	std::istringstream is {str, std::ios::binary};
	{
		std::uint64_t i2 {~i};
		is.read(reinterpret_cast<char*>(&i2), sizeof(i)); // NOLINT(*-cast)
		REQUIRE_FALSE(is.fail());
		CHECK(i == i2);
	}
	// try seeking the stream backward and extracting it again:
	is.seekg(0u);
	{
		std::uint64_t i2 {~i};
		is.read(reinterpret_cast<char*>(&i2), sizeof(i)); // NOLINT(*-cast)
		REQUIRE_FALSE(is.fail());
		CHECK(i == i2);
	}
}


TEST_CASE("okiidoku.mixed_radix_uint_serdes") {
	// okiidoku::test_mixed_radix_uint_serdes<std::uint_least8_t, std::uint_least16_t>(3316611681754028984uLL, 10u);
	static constexpr std::uintmax_t num_rounds {1024u};
	// std::mt19937 rng {0u};
	std::mt19937 rng {std::random_device{}()};
	for (std::uintmax_t round {0u}; round < num_rounds; ++round) { CAPTURE(round);
		using u8_t  = std::uint_least8_t ;
		using u16_t = std::uint_least16_t;
		using u32_t = std::uint_least32_t;
		using u64_t = std::uint_least64_t;
		okiidoku::test::test_mixed_radix_uint_serdes_one_round<u8_t, u8_t >(rng()); // TODO unwrap these to each their own TEST_CASE
		okiidoku::test::test_mixed_radix_uint_serdes_one_round<u8_t, u16_t>(rng());
		okiidoku::test::test_mixed_radix_uint_serdes_one_round<u8_t, u32_t>(rng());
		okiidoku::test::test_mixed_radix_uint_serdes_one_round<u8_t, u64_t>(rng());
	}
}