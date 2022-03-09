add_library(solvent SHARED
	grid.cpp
	print.cpp
	morph/canon_label.cpp
	morph/canon_place.cpp
	morph/canon.cpp
	morph/scramble.cpp
	gen/path.cpp
	gen/backtracking.cpp
	gen/batch.cpp
	gen/batch_print.cpp
)

# https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
include(GenerateExportHeader)
generate_export_header(solvent)
set_target_properties(solvent PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(solvent PROPERTIES CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

find_package(Threads REQUIRED)

target_include_directories(solvent PRIVATE ..)
target_include_directories(solvent PUBLIC ../../include ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(solvent PUBLIC Threads::Threads)
target_link_libraries(solvent PRIVATE solvent_util)