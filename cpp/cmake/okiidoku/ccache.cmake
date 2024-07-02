# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)

# do not wrap this with `block()`
if(okiidoku_IS_TOP_LEVEL AND NOT DEFINED CMAKE_CXX_COMPILER_LAUNCHER)
	find_program(CCACHE_EXE NAMES ccache DOC "Path to ccache executable")
	if(NOT "${CCACHE_EXE}" STREQUAL "CCACHE_EXE-NOTFOUND")
		set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXE}")
	endif()
endif()