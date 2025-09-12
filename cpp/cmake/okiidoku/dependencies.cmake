# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)

# https://github.com/cpm-cmake/CPM.cmake/releases
# copy the latest release's get_cpm.cmake file
include(okiidoku/get_cpm)

# Note: CPM does auto EXCLUDE_FROM_ALL and SYSTEM when using shorthand add

# https://docs.github.com/en/repositories/working-with-files/using-files/downloading-source-code-archives#source-code-archive-urls

CPMAddPackage(NAME range-v3
	# https://github.com/ericniebler/range-v3/tags
	URL [[https://github.com/ericniebler/range-v3/archive/refs/tags/0.12.0.tar.gz]]
	DOWNLOAD_ONLY YES
)
if(range-v3_ADDED)
	add_library(range-v3 INTERFACE IMPORTED)
	add_library(range-v3::range-v3 ALIAS range-v3)
	target_include_directories(range-v3 SYSTEM INTERFACE "${range-v3_SOURCE_DIR}/include")
	# original CMake options I might care about:
	#  RANGES_MODULES, RANGES_POLLY, RANGES_DEEP_STL_INTEGRATION
endif()


CPMAddPackage(NAME pcg
	# https://github.com/imneme/pcg-cpp/commits/master
	# https://github.com/imneme/pcg-cpp/issues/73 new release hasn't been made in a long time
	# https://www.pcg-random.org/using-pcg-cpp.html
	URL [[https://github.com/imneme/pcg-cpp/archive/428802d1a5634f96bcd0705fab379ff0113bcf13.tar.gz]] # take latest
	DOWNLOAD_ONLY YES
)
if(pcg_ADDED)
	add_library(pcg INTERFACE IMPORTED)
	add_library(pcg::pcg ALIAS pcg)
	target_include_directories(pcg SYSTEM INTERFACE "${pcg_SOURCE_DIR}/include")
endif()


if(OKIIDOKU_BUILD_TESTING)
	# https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md
	CPMAddPackage(NAME doctest
		# https://github.com/doctest/doctest/blob/master/CHANGELOG.md
		URL [[https://github.com/doctest/doctest/archive/refs/tags/v2.4.12.tar.gz]] # take latest
		DOWNLOAD_ONLY YES
	)
	if(doctest_ADDED)
		add_library(doctest INTERFACE IMPORTED)
		add_library(doctest::doctest ALIAS doctest)
		target_include_directories(doctest SYSTEM INTERFACE "${doctest_SOURCE_DIR}/doctest")
	endif()
	include("${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake")
endif()


if(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON)
	# CPMAddPackage("gh:pybind/pybind11@3.0.1")
	# https://pybind11.readthedocs.io/en/stable/index.html
	# https://pybind11.readthedocs.io/en/stable/changelog.html

	find_package(Python COMPONENTS Interpreter Development.Module REQUIRED) # https://nanobind.readthedocs.io/en/latest/building.html#preliminaries
	# https://github.com/wjakob/nanobind
	# https://nanobind.readthedocs.io/en/latest/changelog.html
	CPMAddPackage("gh:wjakob/nanobind@2.9.2")
		# git tag archive link doesn't work since submodule deps are required
		# TODO https://github.com/wjakob/nanobind/issues/403 could run `git submodule status` to get robin_map commit and then fetch that tarball there.
		#  https://nanobind.readthedocs.io/en/latest/api_cmake.html#submodule-dependencies https://github.com/wjakob/nanobind/blob/master/cmake/nanobind-config.cmake#L249
	set(NB_STABLE_ABI "")
	set(NB_CORE_LIBNAME nanobind)
	if(NOT OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE)
		set(NB_STABLE_ABI STABLE_API) # stable across minor python versions. small perf cost.
		string(APPEND NB_CORE_LIBNAME "-abi3")
	endif()
	# define libraries here, instead of in downstream subdirectories
	# https://nanobind.readthedocs.io/en/latest/api_cmake.html#command:nanobind_build_library
	nanobind_build_library("${NB_CORE_LIBNAME}" AS_SYSINCLUDE)
	unset(NB_CORE_LIBNAME)
endif()


if(OKIIDOKU_BUILD_DOCS)
	CPMAddPackage(NAME doxygen-awesome-css
		# https://github.com/jothepro/doxygen-awesome-css/releases
		URL [[https://github.com/jothepro/doxygen-awesome-css/archive/refs/tags/v2.3.4.tar.gz]]
		DOWNLOAD_ONLY YES
	)
	if(doxygen-awesome-css_ADDED)
		list(APPEND DOXYGEN_HTML_EXTRA_STYLESHEET
			"${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome.css"
			"${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome-sidebar-only.css"
		)
		# https://github.com/jothepro/doxygen-awesome-css?tab=readme-ov-file#choosing-a-layout
		set(DOXYGEN_DISABLE_INDEX NO)
		set(DOXYGEN_GENERATE_TREEVIEW YES)
		set(DOXYGEN_FULL_SIDEBAR NO)
		set(DOXYGEN_HTML_COLORSTYLE "LIGHT")
	endif()
endif()