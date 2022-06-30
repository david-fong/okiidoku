#include <catch2/catch_test_macros.hpp>

#include <okiidoku/grid.hpp>
#include <okiidoku/ints.hpp>

template<okiidoku::Order O>
void test_ints() {
	using namespace ::okiidoku;
	using namespace ::okiidoku::mono;
	OKIIDOKU_MONO_INT_TS_TYPEDEFS
}

TEST_CASE("algorithms") {
	REQUIRE(true);
}