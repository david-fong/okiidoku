# todo: learn how to do functions/other mechanisms to share config between test targets
add_executable(okiidoku_test tests.cpp)
target_compile_options(okiidoku_test
	PUBLIC
	# -fsanitize=address # ? https://youtu.be/4pKtPWcl1Go?t=12m49s
)
target_include_directories(okiidoku_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(okiidoku_test
	PRIVATE okiidoku_cli_utils
   PRIVATE okiidoku_fmt
	PRIVATE okiidoku_visitor
)

# add_test(NAME test COMMAND okiidoku_test)