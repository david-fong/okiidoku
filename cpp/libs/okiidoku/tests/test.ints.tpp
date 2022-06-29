#include <okiidoku/o2_bit_arr.hpp>
#include <okiidoku/ints.hpp>

template<okiidoku::Order O>
void test_ints() {
	using namespace ::okiidoku;
	using namespace ::okiidoku::mono;
	using T = Ints<O>;
	using o1x_t = int_ts::o1x_t<O>;
	using o2x_t = int_ts::o2x_t<O>;
	using o2i_t = int_ts::o2i_t<O>;
	using o3x_t = int_ts::o3x_t<O>;

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

	for (o2i_t i {0}; i < T::O2; ++i) {
	for (o2i_t j {0}; j < T::O2; ++j) {
		const auto box_cell_rmi {box_cell_to_rmi<O>(i,j)};
		assert(rmi_to_box<O>(box_cell_rmi) == i);
		assert(rmi_to_box_cell<O>(box_cell_rmi) == j);

		const auto chute {static_cast<o1x_t>(i/T::O1)};
		const auto chute_cell {static_cast<o3x_t>(((i%T::O1)*T::O2)+j)};
		const auto h_chute_cell_rmi {chute_cell_to_rmi<O>(LineType::row, chute, chute_cell)};
		const auto v_chute_cell_rmi {chute_cell_to_rmi<O>(LineType::col, chute, chute_cell)};
		assert(h_chute_cell_rmi == row_col_to_rmi<O>(i,j));
		assert(v_chute_cell_rmi == row_col_to_rmi<O>(j,i));
	}}
}