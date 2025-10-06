// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <okiidoku/grid.hpp>
#include <okiidoku/ints_io.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <array>
#include <utility>     // declval
#include <typeinfo>
#include <type_traits>

namespace okiidoku::mono {
	static_assert(std::is_same_v<decltype(std::declval<      Grid<3>  >().get_underlying_array()),       std::array<Int<9,IntKind::small>,81uz>&&>);
	static_assert(std::is_same_v<decltype(std::declval<      Grid<3>&&>().get_underlying_array()),       std::array<Int<9,IntKind::small>,81uz>&&>);
	static_assert(std::is_same_v<decltype(std::declval<      Grid<3>& >().get_underlying_array()),       std::array<Int<9,IntKind::small>,81uz>& >);
	static_assert(std::is_same_v<decltype(std::declval<const Grid<3>& >().get_underlying_array()), const std::array<Int<9,IntKind::small>,81uz>& >);
}

namespace okiidoku::test {
template<Order O> OKIIDOKU_KEEP_FOR_DEBUG // NOLINTNEXTLINE(*-internal-linkage)
void test_grid() {
	using namespace ::okiidoku::mono;
	OKIIDOKU_MONO_INT_TS_TYPEDEFS
	// std::cout<<(typeid(decltype(std::declval<Grid<3>>().get_underlying_array())).name())<<'\n';

	for (const auto i : T::O2) { CAPTURE(i);
	for (const auto j : T::O2) { CAPTURE(j);
		{
			const auto box_cell_rmi {box_cell_to_rmi<O>(i,j)}; CAPTURE(box_cell_rmi);
			CHECK(rmi_to_box<O>(box_cell_rmi) == i);
			CHECK(rmi_to_box_cell<O>(box_cell_rmi) == j);
		}{
			const auto chute {i / T::O1}; CAPTURE(chute);
			const typename T::o3x_t chute_cell {((i%T::O1)*T::O2)+j}; CAPTURE(chute_cell);
			const auto h_chute_cell_rmi {chute_cell_to_rmi<O>(LineType::row, chute, chute_cell)};
			const auto v_chute_cell_rmi {chute_cell_to_rmi<O>(LineType::col, chute, chute_cell)};
			CHECK(h_chute_cell_rmi == row_col_to_rmi<O>(i,j));
			CHECK(v_chute_cell_rmi == row_col_to_rmi<O>(j,i));
		}
	}}

	Grid<O> gen_grid; CAPTURE(gen_grid);
	CHECK(grid_is_empty(gen_grid)); // expected default ctor behaviour

	init_most_canonical_grid(gen_grid);
	CHECK(grid_is_filled(gen_grid));
	CHECK(grid_follows_rule(gen_grid));
}}

TEST_CASE("okiidoku.grid") {
	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
	okiidoku::test::test_grid<(O_)>();
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}