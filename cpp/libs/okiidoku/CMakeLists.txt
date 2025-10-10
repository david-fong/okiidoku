# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later

# https://alexreinking.com/blog/building-a-dual-shared-and-static-library-with-cmake.html
# https://www.reddit.com/r/cpp/comments/m0rt7v/building_a_dual_shared_and_static_library_with/
# TODO.low try Alex's tutorial things for installation utilities.

add_library(okiidoku #[[ uses BUILD_SHARED_LIBS ]] "")
add_library("okiidoku::lib" ALIAS okiidoku)
target_compile_features(okiidoku PUBLIC cxx_std_23)
set_target_properties(okiidoku PROPERTIES
	# https://youtu.be/m0DwB4OvDXk?t=17m13s
	VERSION   "${okiidoku_VERSION}"
	SOVERSION "${okiidoku_VERSION_MAJOR}"
	EXPORT_NAME lib
	CXX_STANDARD_REQUIRED TRUE
	CXX_EXTENSIONS OFF
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN TRUE
	INTERPROCEDURAL_OPTIMIZATION "${okiidoku_is_ipo_supported}"
)
okiidoku_add_compiler_options(okiidoku)
add_subdirectory(src/okiidoku) # target_source_files
okiidoku_install_target(okiidoku)

include(GenerateExportHeader)
generate_export_header(okiidoku EXPORT_FILE_NAME include/okiidoku/detail/export.h)
okiidoku_make_include_flag("okiidoku/detail/export.h" export_header_include_flag)
target_compile_options(okiidoku PUBLIC "${export_header_include_flag}")
if(NOT (DEFINED BUILD_SHARED_LIBS AND BUILD_SHARED_LIBS))
	target_compile_definitions(okiidoku PUBLIC "OKIIDOKU_STATIC_DEFINE")
endif()

include(./cmake/generate_meta_info.cmake)

# MARK: headers
target_include_directories(okiidoku
	PUBLIC  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
	PUBLIC  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
	PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
)
okiidoku_make_include_flag("okiidoku/detail/util.hpp" util_header_include_flag)
target_compile_options(okiidoku PUBLIC "${util_header_include_flag}")
# future note: if switching back to FILE_SET, make sure headers get installed. not sure why they weren't when previously using FILE_SET. maybe each header file needs to be listed out explicitly?
install(DIRECTORY # includes
	"${CMAKE_CURRENT_BINARY_DIR}/include/okiidoku"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/okiidoku"
	TYPE INCLUDE
	COMPONENT okiidoku_development
)

# keep inline functions for debugging purposes:
# TODO clang doesn't support -fkeep-inline-functions, need to use something else... maybe -fno-inline-functions... :(
# TODO do we even want to keep doing this now that we have `OKIIDOKU_KEEP_FOR_DEBUG`?
add_compile_options("$<${debug_configs}:$<$<CXX_COMPILER_ID:GNU>:-fkeep-inline-functions>>")


# MARK: debug support
# too bad can't use execute_process or file(CREATE_LINK) because there's a generator expression in OKIIDOKU_DATA_OUTPUT_DIRECTORY.
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
		add_custom_command(TARGET okiidoku POST_BUILD
			COMMENT "symlinking okiidoku-gdb.py to build directory"
			COMMAND "${c};create_symlink;${tools}/okiidoku-gdb.py;$<TARGET_SONAME_FILE:okiidoku>-gdb.py"
			VERBATIM COMMAND_EXPAND_LISTS
		)
		install(FILES "${tools}/okiidoku-gdb.py"
			RENAME "$<TARGET_SONAME_FILE_NAME:okiidoku>-gdb.py"
			DESTINATION "${CMAKE_INSTALL_DATADIR}/gdb/auto-load/${CMAKE_INSTALL_FULL_LIBDIR}"
			COMPONENT okiidoku_development
		)
	endif()

	# natvis search: https://learn.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects#BKMK_natvis_location
	add_custom_command(TARGET okiidoku POST_BUILD
		COMMENT "symlinking debug-support files to data / build directory"
		BYPRODUCTS
			"${OKIIDOKU_DATA_OUTPUT_DIRECTORY}/okiidoku.natvis"
			"${OKIIDOKU_TEST_WORKING_DIRECTORY}/.gdbinit"
		COMMAND "${c};make_directory;${OKIIDOKU_DATA_OUTPUT_DIRECTORY}/okiidoku"
		COMMAND "${c};create_symlink;${tools}/okiidoku.natvis;${OKIIDOKU_DATA_OUTPUT_DIRECTORY}/okiidoku/okiidoku.natvis"
		COMMAND "${c};create_symlink;${tools}/.gdbinit;${OKIIDOKU_TEST_WORKING_DIRECTORY}/.gdbinit" # local init file (https://sourceware.org/gdb/current/onlinedocs/gdb.html/Initialization-Files.html)
		VERBATIM COMMAND_EXPAND_LISTS
	)
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


# MARK: dependencies
target_link_libraries(okiidoku
	PRIVATE range-v3::range-v3
)

# MARK: examples, tests, pgo
if(OKIIDOKU_BUILD_TESTING)
	add_subdirectory(examples EXCLUDE_FROM_ALL)
	add_subdirectory(tests)
endif()
add_subdirectory(pgo)