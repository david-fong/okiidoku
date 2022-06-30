#include <catch2/catch_test_macros.hpp>

#include <okiidoku/grid.hpp>
#include <okiidoku/ints.hpp>

template<okiidoku::Order O>
void test_grid() {
	using namespace ::okiidoku;
	using namespace ::okiidoku::mono;
	OKIIDOKU_MONO_INT_TS_TYPEDEFS

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

TEST_CASE("grid") {
	REQUIRE(true);
}