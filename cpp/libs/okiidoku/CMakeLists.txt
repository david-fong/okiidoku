# https://alexreinking.com/blog/building-a-dual-shared-and-static-library-with-cmake.html
# https://www.reddit.com/r/cpp/comments/m0rt7v/building_a_dual_shared_and_static_library_with/
# TODO.low there's a lot of other good things in the above tutorial I'm not currently using
add_library(okiidoku
	src/okiidoku/grid.cpp
	src/okiidoku/gen.cpp
	src/okiidoku/fmt/emoji.cpp
	src/okiidoku/fmt/print_2d.cpp
	src/okiidoku/db/serdes.cpp
	src/okiidoku/morph/transform.cpp
	src/okiidoku/morph/scramble.cpp
	src/okiidoku/morph/rel_info.cpp
	src/okiidoku/morph/canon_label.cpp
	src/okiidoku/morph/canon_place.cpp
	src/okiidoku/morph/canon.cpp
	src/okiidoku/puzzle/solve.cpp
)
add_library(okiidoku::okiidoku ALIAS okiidoku)
set_target_properties(okiidoku PROPERTIES
	# https://www.youtube.com/watch?v=m0DwB4OvDXk&ab_channel=CppCon&t=17m13s
	VERSION   ${okiidoku_VERSION}
	SOVERSION ${okiidoku_VERSION_MAJOR}
)

# https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
include(GenerateExportHeader)
generate_export_header(okiidoku EXPORT_FILE_NAME include/okiidoku/detail/export.h)
target_compile_definitions(okiidoku PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:OKIIDOKU_STATIC_DEFINE>")
target_sources(okiidoku PUBLIC FILE_SET HEADERS BASE_DIRS "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>")

target_sources(okiidoku PUBLIC FILE_SET HEADERS BASE_DIRS include)
target_include_directories(okiidoku PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

# find_package(Threads REQUIRED)
target_link_libraries(okiidoku
	PRIVATE CONAN_PKG::range-v3
	# PRIVATE Threads::Threads
)

if(PROJECT_IS_TOP_LEVEL)
	add_subdirectory(examples)
	if(BUILD_TESTING)
		add_subdirectory(tests)
	endif()
endif()