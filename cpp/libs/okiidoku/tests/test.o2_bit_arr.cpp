#include <catch2/catch_test_macros.hpp>

#include <okiidoku/o2_bit_arr.hpp>
#include <okiidoku/ints.hpp>

template<okiidoku::Order O>
void test_ints() {
	using namespace ::okiidoku;
	using namespace ::okiidoku::mono;
	OKIIDOKU_MONO_INT_TS_TYPEDEFS

	assert(O2BitArr_ones<O>.count() == T::O2);
	for (o2i_t i {0}; i < T::O2; ++i) {
		assert(O2BitArr_ones<O>.count_set_bits_below(static_cast<o2x_t>(i)) == i);
		assert(O2BitArr_ones<O>.get_index_of_nth_set_bit(static_cast<o2x_t>(i)) == i);
	}
	{
		auto ones {O2BitArr_ones<O>};
		for (o2i_t i {0}; i < T::O2; ++i) {
			assert(ones.count_set_bits_below(static_cast<o2x_t>(i)) == 0);
			assert(ones.get_index_of_nth_set_bit(0) == i);
			assert(ones.test(ones.count_lower_zeros_assuming_non_empty_mask()));
			ones.unset(ones.count_lower_zeros_assuming_non_empty_mask());
		}
	}
	{
		o2i_t count {0};
		for (auto walker {O2BitArr_ones<O>.set_bits_walker()}; walker.has_more(); walker.advance()) {
			assert(walker.value() == count);
			++count;
		}
		assert(count == T::O2);
	}
}