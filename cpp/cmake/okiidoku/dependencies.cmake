# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)

set(CPM_DOWNLOAD_VERSION 0.35.1) # https://github.com/cpm-cmake/CPM.cmake/tags
include(okiidoku/dependencies.get_cpm)

CPMAddPackage(
	NAME range-v3
	GIT_TAG 0.12.0 # https://github.com/ericniebler/range-v3/releases
	GITHUB_REPOSITORY "ericniebler/range-v3"
	DOWNLOAD_ONLY TRUE
)
if(range-v3_ADDED)
	add_library(range-v3 INTERFACE IMPORTED)
	add_library(range-v3::range-v3 ALIAS range-v3)
	target_include_directories(range-v3 SYSTEM INTERFACE "${range-v3_SOURCE_DIR}/include")
	# original CMake options I might care about:
	#  RANGES_MODULES, RANGES_POLLY, RANGES_DEEP_STL_INTEGRATION
endif()


if(okiidoku_IS_TOP_LEVEL)
	# https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md
	CPMAddPackage(
		NAME catch2
		VERSION 3.1.0
		GITHUB_REPOSITORY "catchorg/Catch2"
		EXCLUDE_FROM_ALL TRUE
	)
	include("${catch2_SOURCE_DIR}/extras/Catch.cmake")
endif()


if(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON)
	CPMAddPackage("gh:pybind/pybind11@2.9.2")
	# https://pybind11.readthedocs.io/en/stable/index.html
	# https://pybind11.readthedocs.io/en/stable/changelog.html
	# Note: CPM does auto EXCLUDE_FROM_ALL when using shorthand add
endif()