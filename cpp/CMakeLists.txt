# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.27)
project(okiidoku
	VERSION 0.0.1
	DESCRIPTION "david fong's variable-grid-size sudoku toolkit"
	HOMEPAGE_URL "https://github.com/david-fong/okiidoku"
	LANGUAGES CXX
)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(OKIIDOKU_VERSION_COMPATIBILITY ExactVersion) # see `cmake/installation.cmake`.
# ^ I will consider switching to support `SameMajorVersion` starting at v1.0.0.

include(okiidoku/default_output_dirs)

include(okiidoku/options) # see this file for more info & defaults. summary:
# OKIIDOKU_BUILD_SHARED_LIBS
# OKIIDOKU_BUILD_WITH_STATIC_ANALYZERS
# OKIIDOKU_BUILD_DEBUG_WITH_SANITIZERS
# OKIIDOKU_BUILD_TESTING
# OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE
# OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON

include(okiidoku/pgo_helpers)
okiidoku_pgo_gen_check_needs_short_circuit()

include(okiidoku/compile_opts.basic)
include(CheckIPOSupported)
check_ipo_supported(RESULT okiidoku_is_ipo_supported)
if(NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION "${okiidoku_is_ipo_supported}")
endif()

if(OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE)
	include(okiidoku/optional/compile_opts.global.non_portable)
endif()
if(OKIIDOKU_BUILD_DEBUG_WITH_SANITIZERS)
	include(okiidoku/optional/sanitizers)
endif()

include(okiidoku/dependencies)

if(OKIIDOKU_BUILD_WITH_STATIC_ANALYSIS)
	# Note: these are done _after_ adding dependencies to avoid doing this work on them.
	include(okiidoku/optional/static_analyzers)
endif()
if(OKIIDOKU_BUILD_TESTING)
	enable_testing() # not `include(CTest)` because I don't use CDash.
endif()


add_subdirectory(libs/okiidoku)
add_subdirectory(libs/cli_utils EXCLUDE_FROM_ALL) # internal

add_subdirectory(apps/cli EXCLUDE_FROM_ALL) # target: okiidoku_cli

add_subdirectory(tools/okiidoku_translator EXCLUDE_FROM_ALL)

if(EMSCRIPTEN)
	add_subdirectory(extras/emscripten) # target: okiidoku_js
endif()
if(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON)
	add_subdirectory(extras/python) # target: okiidoku_py
endif()

okiidoku_enable_profile_guided_optimization(okiidoku #[[trainer:]] okiidoku_pgo_trainer_default)


include(okiidoku/installation)
if(NOT OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE)
	include(okiidoku/packaging)
else()
	message(NOTICE "note that okiidoku packaging rules are not enabled due to using `OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE`.")
endif()