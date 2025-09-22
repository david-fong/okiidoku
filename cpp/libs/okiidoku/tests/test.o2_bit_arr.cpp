// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <okiidoku/o2_bit_arr.hpp>
#include <okiidoku/o2_bit_arr.cpp> // NOLINT(*include*) implementations aren't exported in libokiidoku.
#include <okiidoku/ints_io.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <iostream>

namespace okiidoku {
template<okiidoku::Order O> OKIIDOKU_KEEP_FOR_DEBUG // NOLINTNEXTLINE(*-internal-linkage)
void test_o2_bit_arr() {
	using namespace ::okiidoku::mono;
	OKIIDOKU_MONO_INT_TS_TYPEDEFS

	std::cout << "0" << std::endl;
	CHECK(O2BitArr_ones<O>.count() == T::O2);
	for (const auto i : T::O2) {
		CHECK(O2BitArr_ones<O>.count_below(i) == i);
		CHECK(O2BitArr_ones<O>.get_index_of_nth_set_bit(i) == i);
	}
	std::cout << "1" << std::endl;
	{
		O2BitArr<O> arr {};
		for (const auto i : T::O2) {
			CHECK(!arr[i]);
			arr.set(i);
			CHECK(arr[i]);
		}
	}
	std::cout << "2" << std::endl;
	{
		auto ones {O2BitArr_ones<O>};
		for (const auto i : T::O2) {
			CHECK(ones.count_below(i) == 0u);
			CHECK(ones.get_index_of_nth_set_bit(0u) == i);
			CHECK(ones[ones.first_set_bit_require_exists()]);
			ones.unset(ones.first_set_bit_require_exists());
		}
	}
	std::cout << "3" << std::endl;
	{
		o2i_t count {0};
		for (const auto val : O2BitArr_ones<O>.set_bits()) {
			CHECK(val == count);
			++count;
		}
		CHECK(count == T::O2);
	}
	std::cout << "4" << std::endl;
}}

TEST_CASE("okiidoku.o2_bit_arr") {
	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
	okiidoku::test_o2_bit_arr<O_>();
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}