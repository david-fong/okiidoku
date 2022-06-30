function(okiidoku_add_test name)
	set(target okiidoku_test.${name})
	add_executable(${target} test.${name}.cpp)
	target_link_libraries(${target}
		PRIVATE okiidoku_compile_options_private
		PRIVATE okiidoku
		PRIVATE Catch2::Catch2WithMain
	)
	# https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md#customization
	catch_discover_tests(${target}
		PROPERTIES
			FIXTURES_SETUP ${target}
	)
endfunction()

# https://crascit.com/2016/10/18/test-fixtures-with-cmake-ctest/
function(okiidoku_test_a_depends_on_b target_who_requires target_who_setup)
	set_tests_properties(${okiidoku_test.${target_who_requires}_TESTS} PROPERTIES FIXTURES_REQUIRED okiidoku_test.${target_who_setup})
endfunction()


okiidoku_add_test(o2_bit_arr)
# a bit of a hack to be able to test it (library internals).
target_sources(okiidoku_test.o2_bit_arr PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/../src/okiidoku/o2_bit_arr.cpp"
)
target_include_directories(okiidoku_test.o2_bit_arr PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/../src"
)

okiidoku_add_test(grid)
okiidoku_test_a_depends_on_b(grid o2_bit_arr)

okiidoku_add_test(algorithms)
okiidoku_test_a_depends_on_b(algorithms o2_bit_arr)
okiidoku_test_a_depends_on_b(algorithms grid)