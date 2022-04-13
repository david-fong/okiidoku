add_library(okiidoku_fmt SHARED
	src/okiidoku/fmt/emoji.cpp
	src/okiidoku/fmt/print_2d.cpp
)

# https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
include(GenerateExportHeader)
generate_export_header(okiidoku_fmt BASE_NAME okiidoku)

target_sources(okiidoku_fmt PUBLIC FILE_SET HEADERS BASE_DIRS include)
target_include_directories(okiidoku_fmt
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_link_libraries(okiidoku_fmt
	PUBLIC okiidoku_shared_rng
)