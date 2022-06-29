# TODO.low it would be better to actually demonstrate a CMakeLists.txt file
# importing the library as a separate project.

add_executable(okiidoku_example_mono using_mono.main.cpp)
target_link_libraries(okiidoku_example_mono
	# you won't have these. I only use them to help build the example.
	PRIVATE okiidoku_compile_options_private
	PRIVATE okiidoku_cli_utils

	# you need this!
	PRIVATE okiidoku::okiidoku
)

# TODO.low consider making an example using std::async.
# find_package(Threads REQUIRED)
# target_link_libraries(okiidoku_example_threads
# 	PRIVATE okiidoku_compile_options_private
# 	PRIVATE okiidoku_cli_utils
# 	PRIVATE okiidoku::okiidoku
# 	PRIVATE Threads::Threads
# )