# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.22) # TODO.wait https://gitlab.kitware.com/cmake/cmake/-/issues/23451
project(okiidoku
	VERSION 0.0.1
	DESCRIPTION "david fong's variable-grid-size sudoku toolkit"
	HOMEPAGE_URL "https://github.com/david-fong/okiidoku"
	LANGUAGES CXX
)
list(APPEND CMAKE_MODULE_PATH "${okiidoku_SOURCE_DIR}/cmake")
set(OKIIDOKU_VERSION_COMPATIBILITY ExactVersion) # see `cmake/installation.cmake`.
# ^ I will consider switching to support `SameMajorVersion` starting at v1.0.0.

include(okiidoku/options) # see this file for build options. summary:
# OKIIDOKU_BUILD_TESTING
# OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE
# OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON
# OKIIDOKU_BUILD_SHARED_LIBS


include(okiidoku/compile_opts.basic)

if(OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE)
	include(okiidoku/if_top_level/compile_opts.global.non_portable)
endif()
if(PROJECT_IS_TOP_LEVEL)
	include(okiidoku/if_top_level/sanitizers)

	include(CheckIPOSupported)
	check_ipo_supported(RESULT CMAKE_INTERPROCEDURAL_OPTIMIZATION)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF) # want fast debug build
endif()

include(okiidoku/dependencies)

if(PROJECT_IS_TOP_LEVEL)
	# Note: these are done _after_ adding dependencies to avoid doing this work on them.
	include(okiidoku/if_top_level/static_analyzers)
endif()
if(OKIIDOKU_BUILD_TESTING)
	enable_testing() # not `include(CTest)` because I don't use CDash.
endif()


add_subdirectory(libs/okiidoku)
add_subdirectory(libs/cli_utils EXCLUDE_FROM_ALL) # internal

add_subdirectory(apps/cli EXCLUDE_FROM_ALL) # target: okiidoku_cli

add_subdirectory(tools/okiidoku_translator EXCLUDE_FROM_ALL)

if(EMSCRIPTEN)
	add_subdirectory(extras/bindings/emscripten) # target: okiidoku_js
endif()
if(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON)
	add_subdirectory(extras/bindings/python) # target: okiidoku_py
endif()


include(okiidoku/installation)
if(OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE)
	include(okiidoku/packaging)
endif()