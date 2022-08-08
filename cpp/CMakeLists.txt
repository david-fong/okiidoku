# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.22) # TODO.wait https://gitlab.kitware.com/cmake/cmake/-/issues/23451
project(okiidoku
	VERSION 0.0.1
	DESCRIPTION "david fong's variable-grid-size sudoku toolkit"
	HOMEPAGE_URL "https://github.com/david-fong/okiidoku"
	LANGUAGES CXX
)
set(OKIIDOKU_VERSION_COMPATIBILITY ExactVersion) # see `cmake/installation.cmake`.
# ^ I will consider switching to support `SameMajorVersion` starting at v1.0.0.

option(OKIIDOKU_BUILD_APP_CLI "build the cli wrapper application" YES)
option(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON "build bindings for python" NO)

list(APPEND CMAKE_MODULE_PATH ${okiidoku_SOURCE_DIR}/cmake)

include(okiidoku/compile_opts.basic)

if(PROJECT_IS_TOP_LEVEL)
	include(okiidoku/if_top_level/compile_opts.non_portable)
	include(okiidoku/if_top_level/sanitizers)

	include(CheckIPOSupported)
	check_ipo_supported(RESULT CMAKE_INTERPROCEDURAL_OPTIMIZATION)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF) # want fast debug build
endif()

include(okiidoku/dependencies)

if(PROJECT_IS_TOP_LEVEL)
	# Note: these are done _after_ adding dependencies to avoid doing this work on them.
	include(okiidoku/if_top_level/static_analyzers)
	enable_testing() # not `include(CTest)` because I don't use CDash.
endif()
include(okiidoku/detect_target_isa_support)


# General use cases should use BUILD_SHARED_LIBS to choose whether to build
# the library as shared or static. okiidoku_SHARED_LIBS should only be used
# for convenience when it is desired to make an exception just for okiidoku.
#
# DLLs are currently not supported (I had trouble attempting to get it working).
# https://alexreinking.com/blog/building-a-dual-shared-and-static-library-with-cmake.html
if(TARGET_SUPPORTS_SHARED_LIBS)
	if(MSVC)
		set(okiidoku_SHARED_LIBS NO)
	elseif(PROJECT_IS_TOP_LEVEL)
		set(okiidoku_SHARED_LIBS YES)
	endif()
	if(DEFINED okiidoku_SHARED_LIBS)
		set(BUILD_SHARED_LIBS "${okiidoku_SHARED_LIBS}")
		unset(okiidoku_SHARED_LIBS)
	endif()
	include(okiidoku/shared_lib_things)
endif()


# Note: If an optional target requires fetching large dependencies,
#  guard with a cmake option; otherwise, mark as EXCLUDE_FROM_ALL.
add_subdirectory(libs/okiidoku)
add_subdirectory(libs/cli_utils EXCLUDE_FROM_ALL)
add_subdirectory(apps/cli EXCLUDE_FROM_ALL)
add_subdirectory(tools/okiidoku_translator EXCLUDE_FROM_ALL)
if(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON)
	add_subdirectory(extras/bindings/python)
endif()
# Note: emscripten bindings require emscripten's toolchain. see its subdirectory.


include(okiidoku/installation)
include(okiidoku/packaging)