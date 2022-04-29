# TODO.low it would be better to actually demonstrate a CMakeLists.txt file
# importing the library as a separate project.

add_executable(okiidoku_example_mono using_mono.main.cpp)
target_link_libraries(okiidoku_example_mono
	PRIVATE okiidoku_cli_utils
	PRIVATE okiidoku
)

add_executable(okiidoku_example_visitor using_visitor.main.cpp)
target_link_libraries(okiidoku_example_visitor
	PRIVATE okiidoku_cli_utils
	PRIVATE okiidoku
)