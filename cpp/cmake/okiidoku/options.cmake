# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)


# Suggest build types for single-config generators in cmake-gui
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT is_multi_config)
	if(NOT DEFINED CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "cmake build configuration")
	endif()
	if(PROJECT_IS_TOP_LEVEL)
		set_property(CACHE CMAKE_BUILD_TYPE #[[ APPEND ]]
			PROPERTY STRINGS "Debug;RelWithDebInfo;Release;MinSizeRel;PgoUse"
		)
	endif()
endif()
unset(is_multi_config)


# General use cases should use `BUILD_SHARED_LIBS`.
# Use `OKIIDOKU_BUILD_SHARED_LIBS` to make an exception just for okiidoku.
#
# DLLs are currently not supported (I had trouble attempting to get it working).
# https://alexreinking.com/blog/building-a-dual-shared-and-static-library-with-cmake.html
get_property(okiidoku_target_supports_shared_libs GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
if(okiidoku_target_supports_shared_libs)
	if(MSVC)
		set(OKIIDOKU_BUILD_SHARED_LIBS NO CACHE INTERNAL "DLLs are not yet supported")
	elseif(PROJECT_IS_TOP_LEVEL)
		set(OKIIDOKU_BUILD_SHARED_LIBS YES CACHE BOOL "okiidoku build shared libs")
	elseif(DEFINED OKIIDOKU_BUILD_SHARED_LIBS)
		set(OKIIDOKU_BUILD_SHARED_LIBS "${OKIIDOKU_BUILD_SHARED_LIBS}" CACHE BOOL "okiidoku build shared libs")
	endif()

	if(DEFINED OKIIDOKU_BUILD_SHARED_LIBS)
		set(BUILD_SHARED_LIBS "${OKIIDOKU_BUILD_SHARED_LIBS}")
		unset(OKIIDOKU_BUILD_SHARED_LIBS)
	endif()
	include(okiidoku/shared_lib_things) # TODO should this still be included if no shared libs?
else()
	# do nothing. CMake will give an appropriate warning down the line if needed.
endif()
unset(okiidoku_target_supports_shared_libs)


option(OKIIDOKU_BUILD_TESTING "build tests for okiidoku" "${PROJECT_IS_TOP_LEVEL}")


option(
	OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE
	"optimize for the compiling machine (less portable result). disables CPack."
	#[[ default: ]] "${PROJECT_IS_TOP_LEVEL}"
)


if(NOT DEFINED EMSCRIPTEN)
	option(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON "build bindings for python" NO)
else()
	if(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON)
		message(WARNING "no python on the web")
	endif()
	set(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON NO CACHE INTERNAL "no python on the web")
endif()