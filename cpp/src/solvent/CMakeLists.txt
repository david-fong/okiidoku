add_library(solvent SHARED
	grid.cpp
	print.cpp
	morph/canon_label.cpp
	morph/canon_place.cpp
	morph/canon.cpp
	morph/scramble.cpp
	gen/path.cpp
	gen/mod.cpp
	gen/batch.cpp
	gen/batch_print.cpp
)
target_compile_options(solvent
	PRIVATE
	-fvisibility=default
	# ^would like to use default hidden, but that would mean defining macros
	# for supporting many compilers, or writing huge attribute lists. chose
	# default for experimentation and avoiding compiler compat headaches.
)

# https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
include(GenerateExportHeader)
generate_export_header(solvent)

find_package(Threads REQUIRED)

target_include_directories(solvent PRIVATE ..)
target_include_directories(solvent PUBLIC ../../include ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(solvent PUBLIC Threads::Threads)
target_link_libraries(solvent PRIVATE solvent_util)