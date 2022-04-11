add_library(okiidoku_mono SHARED
	src/okiidoku/mono/grid.cpp
	src/okiidoku/mono/db/serdes.cpp
	src/okiidoku/mono/morph/transform.cpp
	src/okiidoku/mono/morph/rel_info.cpp
	src/okiidoku/mono/morph/canon_label.cpp
	src/okiidoku/mono/morph/canon_place.cpp
	src/okiidoku/mono/morph/canon.cpp
	src/okiidoku/mono/morph/scramble.cpp
	src/okiidoku/mono/gen/stochastic.cpp
	src/okiidoku/mono/gen/batch.cpp
)

# https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
include(GenerateExportHeader)
generate_export_header(okiidoku_mono BASE_NAME okiidoku)

target_sources(okiidoku_mono PUBLIC FILE_SET HEADERS BASE_DIRS include)
target_include_directories(okiidoku_mono
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
	PUBLIC
	# $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

find_package(Threads REQUIRED)
target_link_libraries(okiidoku_mono
	# PRIVATE CONAN_PKG::range-v3
	PUBLIC Threads::Threads
)