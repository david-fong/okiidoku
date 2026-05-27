# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 4.3.1)

# https://raw.githubusercontent.com/Kitware/CMake/refs/tags/v4.3.1/Help/dev/experimental.rst
set(CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_DEPENDENCIES [[1942b4fa-b2c5-4546-9385-83f254070067]])
set(CMAKE_EXPERIMENTAL_GENERATE_SBOM               [[ca494ed3-b261-4205-a01f-603c95e4cae0]])
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD              [[451f2fe2-a8a2-47c3-bc32-94786d8fc91b]])

project(okiidoku
	VERSION        0.0.1
	COMPAT_VERSION 0.0.1
	SPDX_LICENSE   [[AGPL-3.0-or-later]]
	DESCRIPTION    [[david fong's variable-grid-size sudoku toolkit]]
	HOMEPAGE_URL   [[https://github.com/david-fong/okiidoku]]
	LANGUAGES      CXX
)
list(APPEND CMAKE_MODULE_PATH "${okiidoku_SOURCE_DIR}/cmake")
# ^consider switching to support `SameMajorVersion` starting at v1.0.0.
set(CMAKE_CXX_SCAN_FOR_MODULES NO) # I'm not using C++ modules... yet. see https://cmake.org/cmake/help/latest/manual/cmake-cxxmodules.7.html, https://crascit.com/2024/04/04/cxx-modules-cmake-shared-libraries/, ccache docs section on C++ Modules if/when transitioning.
include(okiidoku/default_output_dirs)

include(okiidoku/options) #[[
^^^ see this file for more info & defaults.
summary:
- OKIIDOKU_BUILD_SHARED_LIBS
- OKIIDOKU_BUILD_WITH_SUGGESTED_WARNINGS
- OKIIDOKU_BUILD_WITH_STATIC_ANALYZERS
- OKIIDOKU_BUILD_DEBUG_WITH_SANITIZERS
- OKIIDOKU_BUILD_REPRODUCIBLE
- OKIIDOKU_BUILD_TESTING
- OKIIDOKU_BUILD_DOCS
- OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE
- OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON
]]

include(okiidoku/pgo_helpers)

include(okiidoku/build_tools)
include(okiidoku/install_helpers)
include(okiidoku/compile_opts.basic)

include(okiidoku/optional/compile_opts.global.non_portable)
include(okiidoku/optional/sanitizers)

include(okiidoku/dependencies)

# note: these are done _after_ adding dependencies to avoid doing this work on them.
include(okiidoku/optional/static_analyzers)
if(OKIIDOKU_BUILD_TESTING)
	enable_testing() # not `include(CTest)` because I don't use CDash.
endif()
# keep inline functions for debugging purposes:
# TODO clang doesn't support -fkeep-inline-functions, need to use something else... maybe -fno-inline-functions... :(
# TODO do we even want to keep doing this now that we have `OKIIDOKU_KEEP_FOR_DEBUG`? but that doesn't seem to be working...
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options("$<${debug_configs}:-fkeep-inline-functions>")
endif()

add_subdirectory(libs/okiidoku) # target: okiidoku
add_subdirectory(libs/cli_utils EXCLUDE_FROM_ALL) # (internal)
add_subdirectory(apps/cli) # target: okiidoku_cli
add_subdirectory(extras/okiidoku_translator EXCLUDE_FROM_ALL)
if(EMSCRIPTEN)
	add_subdirectory(extras/emscripten) # target: okiidoku_js
endif()
if(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON)
	# add_subdirectory(extras/python) # target: okiidoku_py
	add_subdirectory(extras/nanobind) # target: okiidoku_nb
endif()

include(okiidoku/source_file_random_seed)
include(okiidoku/optional/doxygen)
include(okiidoku/installation)
include(okiidoku/packaging)
