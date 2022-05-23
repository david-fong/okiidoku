# todo: learn how to do functions/other mechanisms to share config between test targets
add_executable(okiidoku_test tests.cpp)
target_include_directories(okiidoku_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(okiidoku_test
	PRIVATE okiidoku_compile_options_private
	PRIVATE okiidoku_cli_utils
   PRIVATE okiidoku
)

# add_test(NAME test COMMAND okiidoku_test)