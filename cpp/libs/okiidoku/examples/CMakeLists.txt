add_executable(okiidoku_example_mono lib.mono.main.cpp)
target_link_libraries(okiidoku_example_mono
	PRIVATE okiidoku_cli_utils
	PRIVATE okiidoku
)

add_executable(okiidoku_example_visitor lib.visitor.main.cpp)
target_link_libraries(okiidoku_example_visitor
	PRIVATE okiidoku_cli_utils
	PRIVATE okiidoku
)