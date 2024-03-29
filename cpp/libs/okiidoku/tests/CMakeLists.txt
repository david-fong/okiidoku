# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# https://github.com/catchorg/Catch2/blob/devel/docs/usage-tips.md
function(okiidoku_test_add name #[[ ARGN are dependencies ]])
	set(target okiidoku_test.${name})
	add_executable(${target} test.${name}.cpp)
	okiidoku_add_compiler_options(${target})
	target_link_libraries(${target}
		PRIVATE okiidoku
		PRIVATE okiidoku_cli_utils
		PRIVATE doctest::doctest
	)
	target_include_directories(${target} PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/../src"
	)
	# https://crascit.com/2016/10/18/test-fixtures-with-cmake-ctest/
	foreach(dependency ${ARGN})
		list(APPEND dependencies okiidoku_test.${dependency})
	endforeach()

	# https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md#customization
	doctest_discover_tests(${target}
		PROPERTIES # https://cmake.org/cmake/help/latest/manual/cmake-properties.7.html#properties-on-tests
			FIXTURES_SETUP ${target}
			FIXTURES_REQUIRED "${dependencies}"
	)
endfunction()


okiidoku_test_add(o2_bit_arr #[[ DEPS ]])
target_sources(okiidoku_test.o2_bit_arr PRIVATE
	# a bit of a hack to be able to test it (library internals: hidden symbols).
	"${CMAKE_CURRENT_LIST_DIR}/../src/okiidoku/o2_bit_arr.cpp"
)

okiidoku_test_add(grid #[[ DEPS ]] o2_bit_arr)

okiidoku_test_add(morph #[[ DEPS ]] o2_bit_arr grid)

okiidoku_test_add(puzzle #[[ DEPS ]] o2_bit_arr grid)