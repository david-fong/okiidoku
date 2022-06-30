# todo: learn how to do functions/other mechanisms to share config between test targets
add_executable(okiidoku_test tests.cpp)
target_sources(okiidoku_test PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/../src/okiidoku/o2_bit_arr.cpp" # hack to be able to test it.
)
target_include_directories(okiidoku_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(okiidoku_test
	PRIVATE okiidoku_compile_options_private
	PRIVATE okiidoku_cli_utils
   PRIVATE okiidoku
)
target_include_directories(okiidoku_test PRIVATE ../src/)

# add_test(NAME test COMMAND okiidoku_test)

add_executable(okiidoku_test_catch2 tests.catch2.cpp)
target_link_libraries(okiidoku_test_catch2
	# PRIVATE okiidoku_compile_options_private
	PRIVATE Catch2::Catch2WithMain
	PRIVATE okiidoku_cli_utils
   PRIVATE okiidoku
)
catch_discover_tests(okiidoku_test_catch2) # https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md#customization