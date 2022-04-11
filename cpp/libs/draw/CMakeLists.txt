add_library(okiidoku_draw SHARED
	src/okiidoku/draw/emoji.cpp
	src/okiidoku/draw/print_2d.cpp
)

target_include_directories(okiidoku_draw
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
	PUBLIC
	# $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_sources(okiidoku_draw PUBLIC FILE_SET HEADERS BASE_DIRS include)
