# TODO.low it would be better to actually demonstrate a CMakeLists.txt file
# importing the library as a separate project.

add_executable(okiidoku_example_mono using_mono.main.cpp)
target_link_libraries(okiidoku_example_mono
	# you don't need these to use the library.
	# they are only used here to build the example.
	PRIVATE okiidoku_compile_options_private
	PRIVATE okiidoku_cli_utils

	# you need this!
	PRIVATE okiidoku::okiidoku
)