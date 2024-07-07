# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)

# I don't like relying on tinkered defaults like this, but it makes certain
# things easier like GDB support files. Unfortunately, if you want those
# things, but also want to adjust the output directories, you'll have to
# make the paths work out yourself.

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if("${is_multi_config}")
	set(base "${CMAKE_BINARY_DIR}/build/$<CONFIG>")
else()
	set(base "${CMAKE_BINARY_DIR}/build")
endif()

if(EMSCRIPTEN)
	set(CMAKE_INSTALL_BINDIR .)
	set(CMAKE_INSTALL_LIBDIR .)
	set(CMAKE_INSTALL_DATAROOTDIR .)
	set(CMAKE_INSTALL_DOCDIR .)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${base}")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${base}")
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${base}")
	set(OKIIDOKU_DATA_OUTPUT_DIRECTORY "${okiidoku_BINARY_DIR}")
	unset(base)
	return()
endif()

# if user has set anything, don't set defaults for anything
if(
	(DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY) OR
	(DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY) OR
	(DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY) OR
	(DEFINED CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY) OR
	(DEFINED CMAKE_PDB_OUTPUT_DIRECTORY)
)
	if(NOT DEFINED OKIIDOKU_DATA_OUTPUT_DIRECTORY)
		set(OKIIDOKU_DATA_OUTPUT_DIRECTORY "${okiidoku_BINARY_DIR}")
	endif()
	unset(base)
	return()
endif()

include(GNUInstallDirs)
if("${WIN32}")
	set(CMAKE_INSTALL_LIBDIR2 "${CMAKE_INSTALL_BINDIR}")
else()
	set(CMAKE_INSTALL_LIBDIR2 "${CMAKE_INSTALL_LIBDIR}")
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY     "${base}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY     "${base}/${CMAKE_INSTALL_LIBDIR2}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY     "${base}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY "${base}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_PDB_OUTPUT_DIRECTORY         "${base}/${CMAKE_INSTALL_BINDIR}")

if(NOT DEFINED OKIIDOKU_DATA_OUTPUT_DIRECTORY)
	set(OKIIDOKU_DATA_OUTPUT_DIRECTORY "${base}/${CMAKE_INSTALL_DATADIR}")
endif()

unset(CMAKE_INSTALL_LIBDIR2)
unset(base)
unset(is_multi_config)