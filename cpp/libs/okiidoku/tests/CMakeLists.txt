# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# https://github.com/catchorg/Catch2/blob/devel/docs/usage-tips.md

function(okiidoku_test_add name #[[ ARGN are correctness_deps ]])
	set(target "okiidoku_test.${name}")
	add_executable("${target}" "test.${name}.cpp")
	set_target_properties("${target}" PROPERTIES
		EXPORT_NAME "test_${name}"
		INTERPROCEDURAL_OPTIMIZATION NO # links way faster and runs about same speed
	)
	okiidoku_add_compiler_options("${target}")
	target_compile_options("${target}" PRIVATE "$<$<CXX_COMPILER_ID:GNU>:-fwhole-program>") # TODO... when did I add this? how much slower does it make the build? how much faster does it make tests run? did I forget that I explicitly turned of LTO just a few lines above because it make link run slow? could try adding `$<$<NOT:${debug_configs}>:...>`
	target_link_libraries("${target}"
		PRIVATE okiidoku_cli_utils
		PRIVATE doctest::doctest
	)
	if(EMSCRIPTEN)
		target_link_options("${target}" PRIVATE --executable)
	endif()
	target_include_directories("${target}" PRIVATE
		# cheat- allow to include okiidoku headers without linking to okiidoku
		"${CMAKE_CURRENT_LIST_DIR}/../include"
		"${CMAKE_CURRENT_LIST_DIR}/../src"
	)
	target_compile_definitions("${target}" PRIVATE
		DOCTEST_CONFIG_SUPER_FAST_ASSERTS # https://github.com/doctest/doctest/blob/master/doc/markdown/configuration.md#doctest_config_super_fast_asserts
	)
	# https://crascit.com/2016/10/18/test-fixtures-with-cmake-ctest/
	# tests of things this test depends upon to function correctly.
	# if they fail, this will (probably) fail too.
	set(correctness_deps "")
	foreach(dependency ${ARGN})
		list(APPEND correctness_deps "okiidoku_test.${dependency}")
	endforeach()

	# https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md#customization
	# https://github.com/doctest/doctest/blob/master/scripts/cmake/doctest.cmake#L8
	doctest_discover_tests("${target}"
		WORKING_DIRECTORY "${OKIIDOKU_TEST_WORKING_DIRECTORY}"
		PROPERTIES # https://cmake.org/cmake/help/latest/manual/cmake-properties.7.html#properties-on-tests
			FIXTURES_SETUP "${target}"
			FIXTURES_REQUIRED "${correctness_deps}"
	)
	# TODO is there a way to make dependents short-circuit build if this fails to compile? (I'm thinking of compile-time tests like static_asserts+constexpr)
endfunction()


okiidoku_test_add(mixed_radix_uint_serdes #[[ DEPS ]])
okiidoku_test_add(o2_bit_arr #[[ DEPS ]])

okiidoku_test_add(grid #[[ DEPS ]] o2_bit_arr)
target_link_libraries(okiidoku_test.grid PRIVATE okiidoku)

okiidoku_test_add(serdes #[[ DEPS ]] mixed_radix_uint_serdes o2_bit_arr grid)
target_link_libraries(okiidoku_test.serdes PRIVATE okiidoku)

okiidoku_test_add(morph #[[ DEPS ]] o2_bit_arr grid)
target_link_libraries(okiidoku_test.morph PRIVATE okiidoku)

okiidoku_test_add(puzzle #[[ DEPS ]] o2_bit_arr grid) # TODO does this depend logically on correctness of morph facilities?
target_link_libraries(okiidoku_test.puzzle PRIVATE okiidoku)