#include <catch2/catch_test_macros.hpp>

#include <okiidoku/o2_bit_arr.hpp>
#include <okiidoku/ints.hpp>

template<okiidoku::Order O>
void test_o2_bit_arr() {
	using namespace ::okiidoku;
	using namespace ::okiidoku::mono;
	OKIIDOKU_MONO_INT_TS_TYPEDEFS

	REQUIRE(O2BitArr_ones<O>.count() == T::O2);
	for (o2i_t i {0}; i < T::O2; ++i) {
		REQUIRE(O2BitArr_ones<O>.count_set_bits_below(static_cast<o2x_t>(i)) == i);
		REQUIRE(O2BitArr_ones<O>.get_index_of_nth_set_bit(static_cast<o2x_t>(i)) == i);
	}
	{
		auto ones {O2BitArr_ones<O>};
		for (o2i_t i {0}; i < T::O2; ++i) {
			REQUIRE(ones.count_set_bits_below(static_cast<o2x_t>(i)) == 0);
			REQUIRE(ones.get_index_of_nth_set_bit(0) == i);
			REQUIRE(ones.test(ones.count_lower_zeros_assuming_non_empty_mask()));
			ones.unset(ones.count_lower_zeros_assuming_non_empty_mask());
		}
	}
	{
		o2i_t count {0};
		for (auto walker {O2BitArr_ones<O>.set_bits_walker()}; walker.has_more(); walker.advance()) {
			REQUIRE(walker.value() == count);
			++count;
		}
		REQUIRE(count == T::O2);
	}
}

TEST_CASE("o2_bit_arr") {
	#define OKIIDOKU_FOR_COMPILED_O(O_) \
	test_o2_bit_arr<O_>();
	OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
	#undef OKIIDOKU_FOR_COMPILED_O
}