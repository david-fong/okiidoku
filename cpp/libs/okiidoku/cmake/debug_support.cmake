# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:dictionaries cpp-refined
include_guard(DIRECTORY)

include(GNUInstallDirs)
block()
	set(c "${CMAKE_COMMAND};-E")
	set(tools "${okiidoku_SOURCE_DIR}/tools")

	# gdb python extension
	# https://sourceware.org/gdb/current/onlinedocs/gdb.html/Which-flavor-to-choose_003f.html
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_sources(okiidoku PRIVATE
			# add `.debug_gdb_scripts` to cover for build as a static lib:
			"$<${debug_configs}:${CMAKE_CURRENT_SOURCE_DIR}/src/okiidoku/debug/gdb_support.cpp>"
			# note: alternatively to the `debug_configs` generator expression, could probably use `NDEBUG` in the cpp file.
		)
	endif()
	if(BUILD_SHARED_LIBS)
		# https://sourceware.org/gdb/current/onlinedocs/gdb.html/objfile_002dgdbdotext-file.html
		# note: I give up trying to cleanly create `$<TARGET_SONAME_FILE:okiidoku>-gdb.py` for the build tree. every option has some bit that doesn't work well.
		install(FILES "${tools}/okiidoku-gdb.py"
			RENAME "$<TARGET_SONAME_FILE_NAME:okiidoku>-gdb.py"
			DESTINATION "${CMAKE_INSTALL_DATADIR}/gdb/auto-load/${CMAKE_INSTALL_FULL_LIBDIR}"
			COMPONENT okiidoku_development
		)
	endif()

	# natvis search: https://learn.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects#BKMK_natvis_location
	# tell Visual Studio IDE that the PDB exists (https://discourse.cmake.org/t/7316/2):
	target_sources(okiidoku PRIVATE "${tools}/okiidoku.natvis") # TODO.low is this needed, given the below link option?
	if(MSVC) # cl or clang-cl
		# embed natvis in PDB https://gitlab.kitware.com/cmake/cmake/-/issues/16874
		# https://learn.microsoft.com/en-us/cpp/build/reference/natvis-add-natvis-to-pdb
		target_link_options(okiidoku PRIVATE "/NATVIS:${tools}/okiidoku.natvis")
		# TODO verify whether ^this even works or not. see https://gitlab.kitware.com/cmake/cmake/-/issues/23215
		# https://gitlab.kitware.com/cmake/cmake/-/issues/26278#note_1586578

		install(FILES "$<TARGET_PDB_FILE:okiidoku>"
			DESTINATION "${CMAKE_INSTALL_LIBDIR2}"
			COMPONENT okiidoku_development OPTIONAL
		)
	endif()
endblock()