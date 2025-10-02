# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)

# TODO.wait https://gitlab.kitware.com/cmake/cmake/-/issues/26901#note_1652538
# note: can't use CMAKE_CXX_COMPILER_ID because this is pre-language-activation

if(NOT EMSCRIPTEN)

# lld. works with most platforms+toolchains.
if(NOT DEFINED CMAKE_LINKER_TYPE)
	find_program(LLD_EXE NAMES lld DOC "Path of lld executable")
	if(NOT "${LLD_EXE}" STREQUAL "LLD_EXE-NOTFOUND")
		set(CMAKE_LINKER_TYPE LLD CACHE STRING [[https://cmake.org/cmake/help/latest/variable/CMAKE_LINKER_TYPE.html]])
	endif()
endif()

# mold. disappointingly, seems slower for release builds and only slightly faster than Gold for debug.
if(NOT DEFINED CMAKE_LINKER_TYPE AND LINUX)
	# find_program(MOLD_EXE NAMES mold DOC "Path to mold executable")
	# if(NOT "${MOLD_EXE}" STREQUAL "MOLD_EXE-NOTFOUND" AND LINUX)
	# 	set(CMAKE_LINKER_TYPE MOLD CACHE STRING [[https://cmake.org/cmake/help/latest/variable/CMAKE_LINKER_TYPE.html]])
	# 	set(CMAKE_JOB_POOLS "compile=7" "link=1" CACHE STRING "")
	# 	set(CMAKE_JOB_POOL_COMPILE "compile" CACHE STRING "")
	# 	set(CMAKE_JOB_POOL_LINK "link" CACHE STRING "")
	# 	return()
	# endif()
endif()

# gold. RIP (binutils 2.44). https://lwn.net/Articles/1007541/
# if(NOT DEFINED CMAKE_LINKER_TYPE AND LINUX)
# 	find_program(GOLD_EXE NAMES gold DOC "Path of gold executable")
# 	if(NOT "${GOLD_EXE}" STREQUAL "GOLD_EXE-NOTFOUND")
# 		set(CMAKE_LINKER_TYPE GOLD CACHE STRING [[https://cmake.org/cmake/help/latest/variable/CMAKE_LINKER_TYPE.html]])
# 	endif()
# endif()

endif() # if(NOT EMSCRIPTEN)
