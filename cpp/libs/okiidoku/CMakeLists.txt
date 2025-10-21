# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(okiidoku #[[ uses BUILD_SHARED_LIBS ]] "")
add_library("okiidoku::lib" ALIAS okiidoku)
target_compile_features(okiidoku PUBLIC cxx_std_23)
set_target_properties(okiidoku PROPERTIES
	# https://youtu.be/m0DwB4OvDXk?t=17m13s
	VERSION   "${okiidoku_VERSION}"
	SOVERSION "${okiidoku_VERSION_MAJOR}"
	EXPORT_NAME lib
	SPDX_LICENSE [[AGPL-3.0-or-later]]
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
# future note: if switching back to FILE_SET, make sure headers get installed. not sure why they weren't when previously using FILE_SET. maybe each header file needs to be listed out explicitly?
install(DIRECTORY # includes
	"${CMAKE_CURRENT_BINARY_DIR}/include/okiidoku"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/okiidoku"
	TYPE INCLUDE
	COMPONENT okiidoku_development
)

include(./cmake/debug_support.cmake)

target_link_libraries(okiidoku
	PRIVATE range-v3::range-v3
)

# MARK: examples, tests, pgo
if(OKIIDOKU_BUILD_TESTING)
	add_subdirectory(examples EXCLUDE_FROM_ALL)
	add_subdirectory(tests)
endif()
add_subdirectory(pgo)