# https://github.com/catchorg/Catch2/blob/devel/docs/usage-tips.md
function(okiidoku_test_add name)
	set(target okiidoku_test.${name})
	add_executable(${target} test.${name}.cpp)
	target_link_libraries(${target}
		PUBLIC  okiidoku_compile_options_public
		PRIVATE okiidoku_compile_options_private
		PRIVATE okiidoku
		PRIVATE okiidoku_cli_utils
		PRIVATE Catch2::Catch2WithMain
	)
	target_include_directories(${target} PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/../src"
	)
	# https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md#customization
	catch_discover_tests(${target}
		PROPERTIES # https://cmake.org/cmake/help/latest/manual/cmake-properties.7.html#properties-on-tests
			FIXTURES_SETUP ${target}
	)
endfunction()

# https://crascit.com/2016/10/18/test-fixtures-with-cmake-ctest/
function(okiidoku_test_dep depender dependee)
	set_tests_properties(${okiidoku_test.${depender}_TESTS} PROPERTIES
		FIXTURES_REQUIRED okiidoku_test.${dependee}
	)
endfunction()


okiidoku_test_add(o2_bit_arr)
target_sources(okiidoku_test.o2_bit_arr PRIVATE
	# a bit of a hack to be able to test it (library internals: hidden symbols).
	"${CMAKE_CURRENT_LIST_DIR}/../src/okiidoku/o2_bit_arr.cpp"
)

okiidoku_test_add(grid)
okiidoku_test_dep(grid o2_bit_arr)

okiidoku_test_add(morph)
okiidoku_test_dep(morph o2_bit_arr)
okiidoku_test_dep(morph grid)

okiidoku_test_add(puzzle)
okiidoku_test_dep(puzzle o2_bit_arr)
okiidoku_test_dep(puzzle grid)