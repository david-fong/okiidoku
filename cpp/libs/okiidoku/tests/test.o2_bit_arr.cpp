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
	CHECK(O2BitArr_ones<O>.count() == T::O2);
	for (const auto i : T::O2) { CAPTURE(i);
		CHECK(O2BitArr_ones<O>.count_below(i) == i);
		CHECK(O2BitArr_ones<O>.get_index_of_nth_set_bit(i) == i);
	}
	INFO("part 1");
	{
		O2BitArr<O> arr {}; CAPTURE(arr);
		for (const auto i : T::O2) { CAPTURE(i);
			CHECK(!arr[i]);
			arr.set(i);
			CHECK(arr[i]);
		}
	}
	INFO("part 2");
	{
		auto ones {O2BitArr_ones<O>};
		for (const auto i : T::O2) { CAPTURE(i);
			CHECK(ones.count_below(i) == 0u);
			CHECK(ones.get_index_of_nth_set_bit(0u) == i);
			CHECK(ones[ones.first_set_bit_require_exists()]);
			ones.unset(ones.first_set_bit_require_exists());
		}
	}
	INFO("part 3");
	{
		o2i_t count {0u};
		for (const auto sym : O2BitArr_ones<O>.set_bits()) { CAPTURE(sym);
			CHECK(sym == count);
			++count;
		}
		CHECK(count == T::O2);
	}
	INFO("part 4");
}}

TEST_CASE("okiidoku.o2_bit_arr") {
	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
	okiidoku::test::test_o2_bit_arr<(O_)>();
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}