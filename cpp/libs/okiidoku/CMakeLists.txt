# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later

# https://alexreinking.com/blog/building-a-dual-shared-and-static-library-with-cmake.html
# https://www.reddit.com/r/cpp/comments/m0rt7v/building_a_dual_shared_and_static_library_with/
# TODO.low try Alex's tutorial things for installation utilities.

add_library(okiidoku #[[ uses BUILD_SHARED_LIBS ]] "")
add_library("${PROJECT_NAME}::okiidoku" ALIAS okiidoku)
target_compile_features(okiidoku PUBLIC cxx_std_20)
set_target_properties(okiidoku PROPERTIES
	# https://www.youtube.com/watch?v=m0DwB4OvDXk&ab_channel=CppCon&t=17m13s
	VERSION   ${okiidoku_VERSION}
	SOVERSION ${okiidoku_VERSION_MAJOR}
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
target_compile_definitions(okiidoku PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:OKIIDOKU_STATIC_DEFINE>")

include(./cmake/generate_meta_info.cmake)

# MARK: headers
target_include_directories(okiidoku
	PUBLIC  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
	PUBLIC  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
	PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
)
# Future Note: if switching back to FILE_SET, make sure headers get installed. not sure why they weren't when previously using FILE_SET. maybe each header file needs to be listed out explicitly?
install(DIRECTORY
	"${CMAKE_CURRENT_BINARY_DIR}/include/okiidoku"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/okiidoku"
	TYPE INCLUDE
	COMPONENT okiidoku_development
)

# keep inline functions for debugging purposes:
# TODO clang doesn't support -fkeep-inline-functions, need to use something else... maybe -fno-inline-functions... :(
add_compile_options("$<$<CONFIG:Debug,RelWithDebInfo>:$<$<CXX_COMPILER_ID:GNU>:-fkeep-inline-functions>>")

# MARK: debug support
# too bad can't use execute_process or file(CREATE_LINK) because there's a generator expression in OKIIDOKU_DATA_OUTPUT_DIRECTORY.
include(GNUInstallDirs)
block()
	set(c "${CMAKE_COMMAND};-E")
	set(src "${okiidoku_SOURCE_DIR}/tools")
	# TODO can this be a build event custom command?
	add_custom_target(okiidoku_copy_debugger_support_files ALL
		"${c};make_directory;${OKIIDOKU_DATA_OUTPUT_DIRECTORY}"
		COMMAND "${c};create_symlink;${src}/okiidoku.gdb.py;${OKIIDOKU_DATA_OUTPUT_DIRECTORY}/okiidoku.gdb.py"
		COMMAND "${c};create_symlink;${src}/okiidoku.natvis;${OKIIDOKU_DATA_OUTPUT_DIRECTORY}/okiidoku.natvis"
		COMMAND "${c};create_symlink;${src}/ninjatracing.py;${okiidoku_BINARY_DIR}/ninjatracing.py" # sigh. I want to automate generating the ninja trace, and I'm hacking install(CODE) to run it after the whole build finishes. with install(CODE|SCRIPT), there's no robust, generalized way to get the project's source directory.
		COMMENT "Symlinking debug-support files to data / build directory"
		VERBATIM COMMAND_EXPAND_LISTS
	)
	install(FILES
		"${src}/okiidoku.gdb.py"
		"${src}/okiidoku.natvis"
		TYPE DATA # see DEFINE_GDB_PY_SCRIPT in gdb_support.cpp
		# also https://learn.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects#BKMK_natvis_location and https://learn.microsoft.com/en-us/cpp/build/reference/natvis-add-natvis-to-pdb so if a PDB is installed with embedded natvis, no need to install natvis separately?
		COMPONENT okiidoku_development
	)
endblock()

# MARK: dependencies
target_link_libraries(okiidoku
	PRIVATE range-v3::range-v3
)

# MARK: examples, tests, pgo
if(OKIIDOKU_BUILD_TESTING AND NOT EMSCRIPTEN)
	add_subdirectory(examples EXCLUDE_FROM_ALL)
	add_subdirectory(tests)
endif()
add_subdirectory(pgo)