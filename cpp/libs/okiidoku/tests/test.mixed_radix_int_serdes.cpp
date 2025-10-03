// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <okiidoku/detail/mixed_radix_int_serdes.hpp>

#include <fstream>
#include <sstream>
#include <random>
#include <algorithm> // min
#include <string>
#include <array>
#include <cstdint>
#include <climits>   // CHAR_BIT
#include <cstddef>   // size_t
#include <concepts>  // unsigned_integral

namespace okiidoku { namespace {
template<class T> using uidist_t = std::uniform_int_distribution<T>;

template <detail::Radix radix_t, std::unsigned_integral buf_t = std::uintmax_t>
OKIIDOKU_KEEP_FOR_DEBUG
void test_mixed_radix_int_serdes(
	const std::uintmax_t rng_seed,
	[[maybe_unused]] const std::uintmax_t round // for conditional breakpoints
) {
	CAPTURE(rng_seed);
	INFO("sizeof(radix_t) := ", sizeof(radix_t));
	INFO("sizeof(buf_t)   := ", sizeof(buf_t));
	using writer_t = detail::MixedRadixUintWriter<radix_t,buf_t>;
	using reader_t = detail::MixedRadixUintReader<radix_t,buf_t>;
	std::minstd_rand rng {static_cast<std::uint_fast32_t>(rng_seed)};

	// TODO write and read multiple ints to the same stringstream per round.

	static constexpr std::size_t max_num_places {1024u};
	OKIIDOKU_DEFER_INIT std::array<typename writer_t::Item, max_num_places> places_buf; // NOLINT(*-init)
	const std::size_t num_places {uidist_t<std::size_t>{0u, max_num_places}(rng)}; CAPTURE(num_places);

	std::size_t num_byte_count {0uz};
	const auto written_data {[&,round]{
		(void)round; // for conditional breakpoint
		// serialize data to `std::string`:
		writer_t writer;
		std::ostringstream os;
		std::geometric_distribution<radix_t> radix_dist {1.0/(CHAR_BIT*sizeof(buf_t))};
		for (auto place {0uz}; place < num_places; ++place) { CAPTURE(place);
			auto& p {places_buf[place]};
				p.radix = 0u; do { p.radix = radix_dist(rng); } while (p.radix == 0u);
				p.digit = uidist_t<radix_t>{0u,static_cast<radix_t>(p.radix-1u)}(rng);
			if (!writer.accept(p)) [[unlikely]] {
				writer.flush(os);
			}
			REQUIRE(writer.item_count() == place+1uz);
		}
		writer.flush(os);
		num_byte_count += writer.byte_count();
		return os.str();
	}()};
	REQUIRE(written_data.size() == num_byte_count);
	{
		// de-serialize data and check correctness:
		std::istringstream is {written_data};
		reader_t reader;
		for (auto place {0uz}; place < num_places; ++place) { CAPTURE(place);
			const auto& expected {places_buf[place]};
			const auto digit {reader.read(is, expected.radix)};
			REQUIRE(reader.item_count() == place+1uz);
			REQUIRE(digit == expected.digit);
		}
		reader.finish(is);
		REQUIRE(reader.byte_count() == num_byte_count);
	}
}}}

TEST_CASE("okiidoku.mixed_radix_int_serdes") {
	// okiidoku::test_mixed_radix_int_serdes<std::uint_least8_t, std::uint_least16_t>(3316611681754028984uLL, 10u);
	static constexpr std::uintmax_t num_rounds {1024u};
	// std::mt19937_64 rng {0u};
	std::mt19937_64 rng {std::random_device{}()};
	// TODO write logic to restore from failure_checkpoint file? but how to delete it afterward?... does doctest have a way to hook into a TEST_CASE passing?
	//   also... isn't there an easier way given that I'm only using the mersenne twister to generate seeds? if a round fails, write the seed that was passed to the round. :P way simpler.
	for (std::uintmax_t round {0u}; round < num_rounds; ++round) { CAPTURE(round);
		[[maybe_unused]] const auto rng_bak {rng}; // make a copy of PRNG's state before usage.
		INFO("writing backup of PRNG state for this test round to file ", [&]{ // `INFO` is lazy (only on failure).
			std::string file_path {"okiidoku.mixed_radix_int_serdes.failure_checkpoint."};
				// file_path += std::to_string(sizeof(radix_t)) + "." + std::to_string(sizeof(buf_t)); /// \todo
				file_path += ".txt";
			std::ofstream of {file_path};
			of << rng_bak;
			return file_path;
		}());
		okiidoku::test_mixed_radix_int_serdes<std::uint_least8_t, std::uint_least8_t >(rng(), round); // TODO unwrap these to each their own TEST_CASE
		okiidoku::test_mixed_radix_int_serdes<std::uint_least8_t, std::uint_least16_t>(rng(), round);
		okiidoku::test_mixed_radix_int_serdes<std::uint_least8_t, std::uint_least32_t>(rng(), round);
		okiidoku::test_mixed_radix_int_serdes<std::uint_least8_t, std::uint_least64_t>(rng(), round);
	}
}