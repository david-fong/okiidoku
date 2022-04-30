add_library(okiidoku SHARED # TODO.mid having trouble getting SHARED working with MSVC. workaround is to just use STATIC for MSVC.
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
)

# https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
include(GenerateExportHeader)
generate_export_header(okiidoku BASE_NAME)

target_sources(okiidoku PUBLIC FILE_SET HEADERS BASE_DIRS include)
target_include_directories(okiidoku
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

# find_package(Threads REQUIRED)
# target_link_libraries(okiidoku
	# PRIVATE CONAN_PKG::range-v3
	# PRIVATE Threads::Threads
# )

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	add_subdirectory(examples)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
	add_subdirectory(tests)
endif()