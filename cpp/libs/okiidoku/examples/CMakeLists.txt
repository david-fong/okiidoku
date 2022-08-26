# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later

# TODO.low it would be better to actually demonstrate a CMakeLists.txt file
# importing the library as a separate project.

add_executable(okiidoku_example_mono using_mono.main.cpp)
okiidoku_add_compiler_options(okiidoku_example_mono) # you don't need this.
target_link_libraries(okiidoku_example_mono
	# you won't have these. I only use them to help build the example.
	PRIVATE okiidoku_cli_utils

	# you need this!
	PRIVATE okiidoku::okiidoku
)

# TODO.low consider making an example using std::async.
# find_package(Threads REQUIRED)
# okiidoku_add_compiler_options(okiidoku_example_threads)
# target_link_libraries(okiidoku_example_threads
	# PRIVATE okiidoku_cli_utils
	# PRIVATE okiidoku::okiidoku
	# PRIVATE Threads::Threads
# )