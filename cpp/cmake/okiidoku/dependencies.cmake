# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)

# https://github.com/cpm-cmake/CPM.cmake/releases
include(okiidoku/get_cpm)

# Note: CPM does auto EXCLUDE_FROM_ALL and SYSTEM when using shorthand add
# if("${CPM_SOURCE_CACHE}" PATH_EQUAL "${okiidoku_SOURCE_DIR}/external")
# 	set(CUSTOM_CACHE_KEY CUSTOM_CACHE_KEY "_") # TODO this looks cleaner, but thwarts fetch when version changes (doesn't detect as dirty).
# else()
	set(CUSTOM_CACHE_KEY)
# endif()


CPMAddPackage(NAME range-v3
	# https://github.com/ericniebler/range-v3/tags
	URL [[https://github.com/ericniebler/range-v3/archive/refs/tags/0.12.0.tar.gz]]
	DOWNLOAD_ONLY YES ${CUSTOM_CACHE_KEY}
)
if(range-v3_ADDED)
	add_library(range-v3 INTERFACE IMPORTED)
	add_library(range-v3::range-v3 ALIAS range-v3)
	target_include_directories(range-v3 SYSTEM INTERFACE "${range-v3_SOURCE_DIR}/include")
	# original CMake options I might care about:
	#  RANGES_MODULES, RANGES_POLLY, RANGES_DEEP_STL_INTEGRATION
endif()


if(OKIIDOKU_BUILD_TESTING)
	# https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md
	CPMAddPackage(NAME doctest
		# https://github.com/doctest/doctest/blob/master/CHANGELOG.md
		URL [[https://github.com/doctest/doctest/archive/refs/tags/v2.4.11.tar.gz]]
		DOWNLOAD_ONLY YES ${CUSTOM_CACHE_KEY}
	)
	if(doctest_ADDED)
		add_library(doctest INTERFACE IMPORTED)
		add_library(doctest::doctest ALIAS doctest)
		target_include_directories(doctest SYSTEM INTERFACE "${doctest_SOURCE_DIR}")
	endif()
	include("${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake")
endif()


if(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON)
	# CPMAddPackage("gh:pybind/pybind11@2.9.2")
	# https://pybind11.readthedocs.io/en/stable/index.html
	# https://pybind11.readthedocs.io/en/stable/changelog.html

	find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
	# https://github.com/wjakob/nanobind
	# https://nanobind.readthedocs.io/en/latest/changelog.html
	# https://nanobind.readthedocs.io/en/latest/building.html#finding-nanobind
	CPMAddPackage("gh:wjakob/nanobind@2.7.0") # TODO ${CUSTOM_CACHE_KEY}
		# git tag archive doesn't work since submodule deps are required
		# https://github.com/wjakob/nanobind/issues/403 could run `git submodule status` to get robin_map commit and then fetch that tarball there.
	foreach(lib "" "-abi3") # https://nanobind.readthedocs.io/en/latest/api_cmake.html#command:nanobind_build_library
		nanobind_build_library("nanobind${lib}")
		set_target_properties("nanobind${lib}" PROPERTIES SYSTEM YES)
	endforeach()
endif()
