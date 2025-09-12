# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)


# Suggest build types for single-config generators in cmake-gui
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT is_multi_config)
	set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "cmake build configuration")
	if(okiidoku_IS_TOP_LEVEL)
		set_property(CACHE CMAKE_BUILD_TYPE #[[ APPEND ]]
			PROPERTY STRINGS "Debug;RelWithDebInfo;Release;MinSizeRel;PgoUse"
		)
	endif()
endif()
unset(is_multi_config)


# OKIIDOKU_BUILD_SHARED_LIBS
# General use cases should use `BUILD_SHARED_LIBS`.
# Use this to make an exception just for okiidoku.
#
# DLLs are currently not supported (I had trouble attempting to get it working).
# https://alexreinking.com/blog/building-a-dual-shared-and-static-library-with-cmake.html
get_property(okiidoku_target_supports_shared_libs GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
if(okiidoku_target_supports_shared_libs)
	if(MSVC)
		set(OKIIDOKU_BUILD_SHARED_LIBS NO CACHE INTERNAL "DLLs are not yet supported")
	elseif(okiidoku_IS_TOP_LEVEL)
		set(OKIIDOKU_BUILD_SHARED_LIBS YES CACHE BOOL "okiidoku build shared libs")
	elseif(DEFINED OKIIDOKU_BUILD_SHARED_LIBS)
		set(OKIIDOKU_BUILD_SHARED_LIBS "${OKIIDOKU_BUILD_SHARED_LIBS}" CACHE BOOL "okiidoku build shared libs")
	endif()

	if(DEFINED OKIIDOKU_BUILD_SHARED_LIBS)
		set(BUILD_SHARED_LIBS "${OKIIDOKU_BUILD_SHARED_LIBS}")
		unset(OKIIDOKU_BUILD_SHARED_LIBS)
	endif()
	include(okiidoku/shared_lib_things)
else()
	# do nothing. CMake will give an appropriate warning down the line if needed.
endif()
unset(okiidoku_target_supports_shared_libs)


option(OKIIDOKU_BUILD_WITH_SUGGESTED_WARNINGS "build okiidoku with suggested warnings" "${okiidoku_IS_TOP_LEVEL}")
option(OKIIDOKU_BUILD_WITH_STATIC_ANALYZERS "build okiidoku with static analyzers" NO) # off by default because it's slow
option(OKIIDOKU_BUILD_DEBUG_WITH_SANITIZERS "build okiidoku debug builds with sanitizers" NO) # default off <- see :/writings/journal/toolchain-papercuts.
option(OKIIDOKU_BUILD_TESTING "build tests and examples for okiidoku" "${okiidoku_IS_TOP_LEVEL}")
option(OKIIDOKU_BUILD_DOCS "build doxygen docs for okiidoku" "${okiidoku_IS_TOP_LEVEL}")


if(NOT EMSCRIPTEN)
option(
	OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE
	"optimize for the compiling machine (less portable result). disables CPack."
	#[[ default: ]] "${okiidoku_IS_TOP_LEVEL}"
)
endif()


if(NOT EMSCRIPTEN)
	option(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON "build bindings for python" NO)
else()
	if(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON)
		message(WARNING "no python on the web")
	endif()
	set(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON NO CACHE INTERNAL "no python on the web")
endif()