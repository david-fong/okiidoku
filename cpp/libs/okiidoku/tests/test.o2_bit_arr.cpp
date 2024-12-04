// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <okiidoku/o2_bit_arr.cpp> // choosing to get implementations this way because they're not exported in libokiidoku.
#include <okiidoku/ints.hpp>

namespace okiidoku {
template<okiidoku::Order O>
[[gnu::noinline]] void test_o2_bit_arr() {
	using namespace ::okiidoku;
	using namespace ::okiidoku::mono;
	OKIIDOKU_MONO_INT_TS_TYPEDEFS

	std::cout << "0" << std::endl;
	CHECK(O2BitArr_ones<O>.count() == T::O2);
	for (o2i_t i {0}; i < T::O2; ++i) {
		CHECK(O2BitArr_ones<O>.count_below(static_cast<o2x_t>(i)) == i);
		CHECK(O2BitArr_ones<O>.get_index_of_nth_set_bit(static_cast<o2x_t>(i)) == i);
	}
	std::cout << "1" << std::endl;
	{
		O2BitArr<O> arr {};
		for (o2i_t i {0}; i < T::O2; ++i) {
			CHECK(!arr.test(static_cast<o2x_t>(i)));
			arr.set(i);
			CHECK(arr.test(i));
		}
	}
	std::cout << "2" << std::endl;
	{
		auto ones {O2BitArr_ones<O>};
		for (o2i_t i {0}; i < T::O2; ++i) {
			CHECK(ones.count_below(static_cast<o2x_t>(i)) == 0);
			CHECK(ones.get_index_of_nth_set_bit(0) == i);
			CHECK(ones.test(ones.first_set_bit_require_exists()));
			ones.unset(ones.first_set_bit_require_exists());
		}
	}
	std::cout << "3" << std::endl;
	{
		o2i_t count {0};
		for (auto walker {O2BitArr_ones<O>.set_bits_walker()}; walker.has_more(); walker.advance()) {
			CHECK(walker.value() == count);
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