# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 4.1)
project(okiidoku_example
	VERSION 0.0.1
	LANGUAGES CXX
)

if(okiidoku_example_IS_TOP_LEVEL)
	find_package(okiidoku CONFIG REQUIRED)

	# alternatively:
	# include(FetchContent)
	# FetchContent_Declare(
	# 	okiidoku
	# 	GIT_REPOSITORY https://github.com/david-fong/okiidoku.git
	# 	# ...
	# 	FIND_PACKAGE_ARGS
	# )
	# # ...
	# FetchContent_MakeAvailable(okiidoku)
endif()


add_executable(okiidoku_example_mono "using_mono.main.cpp")
if(NOT okiidoku_example_IS_TOP_LEVEL)
	okiidoku_add_compiler_options(okiidoku_example_mono) # you don't need this.
endif()
target_link_libraries(okiidoku_example_mono
	PRIVATE okiidoku::lib # <- you need this!

	# you won't have these. I only use them to help build the example.
	PRIVATE okiidoku_cli_utils
)

# TODO.low consider making an example using std::async.
# find_package(Threads REQUIRED) # TODO is this needed for std::async?
# okiidoku_add_compiler_options(okiidoku_example_threads)
# target_link_libraries(okiidoku_example_threads
	# PRIVATE okiidoku_cli_utils
	# PRIVATE okiidoku::lib
	# PRIVATE Threads::Threads
# )