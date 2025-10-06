// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <okiidoku/o2_bit_arr.hpp>
#include <okiidoku/o2_bit_arr.cpp> // NOLINT(*include*) implementations aren't exported in libokiidoku.
#include <okiidoku/ints_io.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#if __has_include(<immintrin.h>)
#include <immintrin.h>
#endif

namespace okiidoku::test {
template<Order O> OKIIDOKU_KEEP_FOR_DEBUG // NOLINTNEXTLINE(*-internal-linkage)
void test_o2_bit_arr() {
	using namespace ::okiidoku::mono;
	OKIIDOKU_MONO_INT_TS_TYPEDEFS

	INFO("part 0");
	REQUIRE_EQ(O2BitArr_ones<O>.count(), T::O2);
	for (const auto i : T::O2) { CAPTURE(i);
		REQUIRE_EQ(O2BitArr_ones<O>.count_below(i), i);
		REQUIRE_EQ(O2BitArr_ones<O>.get_index_of_nth_set_bit(i), i);
	}
	INFO("part 1");
	{
		O2BitArr<O> arr {}; CAPTURE(arr);
		for (const auto i : T::O2) { CAPTURE(i);
			REQUIRE_UNARY(!arr[i]);
			arr.set(i);
			REQUIRE_UNARY(arr[i]);
		}
	}
	INFO("part 2");
	{
		auto ones {O2BitArr_ones<O>};
		for (const auto i : T::O2) { CAPTURE(i);
			REQUIRE_EQ(ones.count_below(i), 0u);
			REQUIRE_EQ(ones.get_index_of_nth_set_bit(0u), i);
			REQUIRE_UNARY(ones[ones.first_set_bit_require_exists()]);
			ones.unset(ones.first_set_bit_require_exists());
		}
	}
	INFO("part 3");
	{
		o2i_t count {0u};
		for (const auto sym : O2BitArr_ones<O>.set_bits()) { CAPTURE(sym);
			REQUIRE_EQ(sym, count);
			++count;
		}
		REQUIRE_EQ(count, T::O2);
	}
	INFO("part 4");
	// TODO test consistency between `first_set_bit_require_exists` and `count_below`. generate some random `O2BitArr`s. to naively get a random number of set bits, start by getting a number of bits to try to set in [0,O2).
}}

TEST_CASE("okiidoku.o2_bit_arr") {
	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
	okiidoku::test::test_o2_bit_arr<(O_)>();
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}