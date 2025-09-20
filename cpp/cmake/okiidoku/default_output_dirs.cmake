# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)

# not really about output dirs, but throwing this in here.
get_property(DEBUG_CONFIGURATIONS GLOBAL PROPERTY DEBUG_CONFIGURATIONS)
list(APPEND DEBUG_CONFIGURATIONS Debug RelWithDebInfo)
set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS "${DEBUG_CONFIGURATIONS}")
set(debug_configs "$<CONFIG:${DEBUG_CONFIGURATIONS}>")
string(REPLACE ";" "," debug_configs "${debug_configs}")
add_compile_definitions("$<$<NOT:${debug_configs}>:NDEBUG>") # CMake handles for its builtin release configs, but not custom ones.

if(NOT okiidoku_IS_TOP_LEVEL)
	return()
endif()

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if("${is_multi_config}")
	set(base "${CMAKE_BINARY_DIR}/build/$<CONFIG>")
else()
	set(base "${CMAKE_BINARY_DIR}/build")
endif()
set (OKIIDOKU_LTO_CACHE_DIR "${base}/lto")

if (NOT DEFINED CMAKE_DEBUGGER_WORKING_DIRECTORY)
	set(CMAKE_DEBUGGER_WORKING_DIRECTORY "${base}")
	# set(CMAKE_VS_DEBUGGER_WORKING_DIRECTORY "${base}") # technically not needed. undocumented behaviour that it falls back to the above: https://github.com/search?q=repo%3AKitware%2FCMake%20%22GetDebuggerWorkingDirectory%22&type=code
endif()

if(EMSCRIPTEN)
	set(CMAKE_INSTALL_BINDIR .)
	set(CMAKE_INSTALL_LIBDIR .)
	set(CMAKE_INSTALL_DATAROOTDIR .)
	set(CMAKE_INSTALL_DOCDIR "doc")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${base}")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${base}")
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${base}")
	set(OKIIDOKU_DATA_OUTPUT_DIRECTORY "${base}")
	unset(base)
	unset(is_multi_config)
	return()
endif()

# if user hasn't set anything, set defaults for everything
if(
	(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY) AND
	(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY) AND
	(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY) AND
	(NOT DEFINED CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY) AND
	(NOT DEFINED CMAKE_PDB_OUTPUT_DIRECTORY)
)
	include(GNUInstallDirs)
	if("${WIN32}")
		set(CMAKE_INSTALL_LIBDIR2 "${CMAKE_INSTALL_BINDIR}") # put DLLs in bin dir
	else()
		set(CMAKE_INSTALL_LIBDIR2 "${CMAKE_INSTALL_LIBDIR}") # put SOs in lib dir
	endif()
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY     "${base}/${CMAKE_INSTALL_BINDIR}")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY     "${base}/${CMAKE_INSTALL_LIBDIR2}")
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY     "${base}/${CMAKE_INSTALL_LIBDIR}")
	set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY "${base}/${CMAKE_INSTALL_BINDIR}")
	set(CMAKE_PDB_OUTPUT_DIRECTORY         "${base}/${CMAKE_INSTALL_BINDIR}")
	set(OKIIDOKU_TEST_WORKING_DIRECTORY    "${base}")
else()
	set(OKIIDOKU_TEST_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()

if(NOT DEFINED OKIIDOKU_DATA_OUTPUT_DIRECTORY)
	set(OKIIDOKU_DATA_OUTPUT_DIRECTORY "${base}/${CMAKE_INSTALL_DATAROOTDIR}")
endif()

unset(CMAKE_INSTALL_LIBDIR2)
unset(base)
unset(is_multi_config)
