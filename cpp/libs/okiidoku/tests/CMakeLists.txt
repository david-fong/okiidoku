# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# https://github.com/catchorg/Catch2/blob/devel/docs/usage-tips.md

function(okiidoku_test_add name #[[ ARGN are correctness_deps ]])
	set(target "okiidoku_test.${name}")
	add_executable("${target}" "test.${name}.cpp")
	set_target_properties("${target}" PROPERTIES
		EXPORT_NAME "test_${name}"
		INTERPROCEDURAL_OPTIMIZATION NO # links way faster and runs about same speed
		FOLDER "okiidoku/test" # for listing in IDE
	)
	okiidoku_add_compiler_options("${target}")
	target_compile_options("${target}" PRIVATE "$<$<NOT:${debug_configs}>:$<$<CXX_COMPILER_ID:GNU>:-fwhole-program>>")
	target_compile_definitions("${target}" PRIVATE
		"CATCH_CONFIG_FAST_COMPILE" # https://github.com/catchorg/Catch2/blob/devel/docs/configuration.md#catch_config_fast_compile
	)
	target_link_libraries("${target}"
		PRIVATE okiidoku_cli_utils
		PRIVATE Catch2::Catch2WithMain
	)
	if(EMSCRIPTEN)
		target_link_options("${target}" PRIVATE --executable)
	endif()
	target_include_directories("${target}" PRIVATE
		# cheat- allow to include okiidoku headers without linking to okiidoku
		"${CMAKE_CURRENT_LIST_DIR}/../include"
		"${CMAKE_CURRENT_LIST_DIR}/../src"
	)
	# https://crascit.com/2016/10/18/test-fixtures-with-cmake-ctest/
	# tests of things this test depends upon to function correctly.
	# if they fail, this will (probably) fail too.
	set(correctness_deps "")
	foreach(dependency ${ARGN})
		list(APPEND correctness_deps "okiidoku_test.${dependency}")
	endforeach()

	# https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md#catch_discover_tests
	# https://github.com/catchorg/Catch2/blob/devel/extras/Catch.cmake#L158
	include(Catch)
	catch_discover_tests("${target}"
		WORKING_DIRECTORY "${OKIIDOKU_TEST_WORKING_DIRECTORY}"
		PROPERTIES # https://cmake.org/cmake/help/latest/manual/cmake-properties.7.html#properties-on-tests
			FIXTURES_SETUP "${target}"
			FIXTURES_REQUIRED "${correctness_deps}"
	)
	# TODO is there a way to make dependents short-circuit build if this fails to compile? (I'm thinking of compile-time tests like static_asserts+constexpr)
endfunction()

set(gdb_support_cpp "${CMAKE_CURRENT_LIST_DIR}/../src/okiidoku/debug/gdb_support.cpp")

okiidoku_test_add(mixed_radix_uint_serdes #[[ DEPS ]])
target_sources(okiidoku_test.mixed_radix_uint_serdes PRIVATE "${gdb_support_cpp}")

okiidoku_test_add(o2_bit_arr #[[ DEPS ]])
target_sources(okiidoku_test.o2_bit_arr PRIVATE "${gdb_support_cpp}")

okiidoku_test_add(grid #[[ DEPS ]] o2_bit_arr)
target_link_libraries(okiidoku_test.grid PRIVATE okiidoku)

okiidoku_test_add(serdes #[[ DEPS ]] mixed_radix_uint_serdes o2_bit_arr grid)
target_link_libraries(okiidoku_test.serdes PRIVATE okiidoku)

okiidoku_test_add(morph #[[ DEPS ]] o2_bit_arr grid)
target_link_libraries(okiidoku_test.morph PRIVATE okiidoku)

okiidoku_test_add(puzzle #[[ DEPS ]] o2_bit_arr grid) # TODO does this depend logically on correctness of morph facilities?
target_link_libraries(okiidoku_test.puzzle PRIVATE okiidoku)