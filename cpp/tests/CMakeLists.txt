# todo: learn how to do functions/other mechanisms to share config between test targets
add_executable(ookiidoku_test tests.cpp)
target_compile_options(ookiidoku_test
	PUBLIC
	# -fsanitize=address # ? https://youtu.be/4pKtPWcl1Go?t=12m49s
)
target_include_directories(ookiidoku_test PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(ookiidoku_test PRIVATE ookiidoku_util ookiidoku)

# add_test(NAME test COMMAND ookiidoku_test)