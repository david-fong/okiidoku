# todo: learn how to do functions/other mechanisms to share config between test targets
add_executable(okiidoku_test tests.cpp)
target_compile_options(okiidoku_test
	PUBLIC
	# -fsanitize=address # ? https://youtu.be/4pKtPWcl1Go?t=12m49s
)
target_include_directories(okiidoku_test PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(okiidoku_test PRIVATE okiidoku_util okiidoku)

# add_test(NAME test COMMAND okiidoku_test)