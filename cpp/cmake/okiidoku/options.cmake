# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)

option(
	OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE
	"optimize for the compiling machine (less portable result). disables CPack."
	"${PROJECT_IS_TOP_LEVEL}" # default
)


if(NOT DEFINED EMSCRIPTEN)
	option(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON "build bindings for python" NO)
else()
	if(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON)
		message(WARNING "no python on the web")
	endif()
	set(OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON NO CACHE INTERNAL "no python on the web")
endif()


# General use cases should use `BUILD_SHARED_LIBS`.
# Use `OKIIDOKU_BUILD_SHARED_LIBS` to make an exception just for okiidoku.
#
# DLLs are currently not supported (I had trouble attempting to get it working).
# https://alexreinking.com/blog/building-a-dual-shared-and-static-library-with-cmake.html
get_property(okiidoku_target_supports_shared_libs GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
if(okiidoku_target_supports_shared_libs)
	if(MSVC)
		set(OKIIDOKU_BUILD_SHARED_LIBS NO)
	elseif(PROJECT_IS_TOP_LEVEL)
		set(OKIIDOKU_BUILD_SHARED_LIBS YES)
	endif()
	if(DEFINED OKIIDOKU_BUILD_SHARED_LIBS)
		set(BUILD_SHARED_LIBS "${OKIIDOKU_BUILD_SHARED_LIBS}")
		unset(OKIIDOKU_BUILD_SHARED_LIBS)
	endif()
	include(okiidoku/shared_lib_things)
endif()
unset(okiidoku_target_supports_shared_libs)