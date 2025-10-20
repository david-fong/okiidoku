// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <catch2/catch_test_macros.hpp>

#include <okiidoku/grid.hpp>
#include <okiidoku/ints_io.hpp>
#include <okiidoku/ints.hpp>
#include <okiidoku/order.hpp>

#include <array>
#include <utility>     // declval, swap
#include <type_traits>

namespace okiidoku::mono {
	static_assert(std::is_same_v<decltype(std::declval<      Grid<3>  >().arr()),       std::array<Int<9,IntKind::small>,81uz>&&>);
	static_assert(std::is_same_v<decltype(std::declval<      Grid<3>&&>().arr()),       std::array<Int<9,IntKind::small>,81uz>&&>);
	static_assert(std::is_same_v<decltype(std::declval<      Grid<3>& >().arr()),       std::array<Int<9,IntKind::small>,81uz>& >);
	static_assert(std::is_same_v<decltype(std::declval<const Grid<3>& >().arr()), const std::array<Int<9,IntKind::small>,81uz>& >);
}

namespace okiidoku::test {
template<Order O> OKIIDOKU_KEEP_FOR_DEBUG // NOLINTNEXTLINE(*-internal-linkage)
void test_grid() {
	INFO("testing for order " << unsigned{O});
	using namespace ::okiidoku::mono;
	OKIIDOKU_MONO_INT_TS_TYPEDEFS
	// std::cout<<(typeid(decltype(std::declval<Grid<3>>().arr).name())<<'\n'; // #include <typeinfo>

	for (const auto i : T::O2) { CAPTURE(i);
	for (const auto j : T::O2) { CAPTURE(j);
		REQUIRE(i == 0u);
		{
			const auto box_cell_rmi {box_cell_to_rmi<O>(i,j)}; CAPTURE(box_cell_rmi);
			REQUIRE(rmi_to_box<O>(box_cell_rmi) == i);
			REQUIRE(rmi_to_box_cell<O>(box_cell_rmi) == j);
		}{
			const auto chute {i / T::O1}; CAPTURE(chute);
			const typename T::o3x_t chute_cell {((i%T::O1)*T::O2)+j}; CAPTURE(chute_cell);
			const auto h_chute_cell_rmi {chute_cell_to_rmi<O>(LineType::row, chute, chute_cell)};
			const auto v_chute_cell_rmi {chute_cell_to_rmi<O>(LineType::col, chute, chute_cell)};
			REQUIRE(h_chute_cell_rmi == row_col_to_rmi<O>(i,j));
			REQUIRE(v_chute_cell_rmi == row_col_to_rmi<O>(j,i));
		}
	}}

	Grid<O> grid; CAPTURE(grid);
	CHECK(grid.is_empty()); // expected default ctor behaviour

	grid.init_most_canonical();
	CHECK(grid.is_filled());
	CHECK(grid.follows_rule());

	std::swap(grid[0u,0u], grid[0u,1u]);
	CHECK(grid.is_filled());
	CHECK_FALSE(grid.follows_rule());

	// put that thing back where it came from, or so help me-
	std::swap(grid[0u,0u], grid[0u,1u]);
	CHECK(grid.is_filled());
	CHECK(grid.follows_rule());
}}

TEST_CASE("okiidoku.grid") {
	#define OKIIDOKU_FOREACH_O_EMIT(O_) \
	okiidoku::test::test_grid<(O_)>();
	OKIIDOKU_FOREACH_O_DO_EMIT
	#undef OKIIDOKU_FOREACH_O_EMIT
}