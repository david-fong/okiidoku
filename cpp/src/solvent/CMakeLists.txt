add_library(solvent SHARED
	rng.cpp
	grid.cpp
	print.cpp
	morph/canon_label.cpp
	morph/canon_place.cpp
	morph/canon.cpp
	morph/scramble.cpp
	gen/path.cpp
	gen/backtracking.cpp
	# gen/stochastic.cpp
	gen/batch.cpp
	gen/batch_print.cpp
)
# https://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html

# https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
include(GenerateExportHeader)
generate_export_header(solvent)
set_target_properties(solvent PROPERTIES
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN TRUE
	# POSITION_INDEPENDENT_CODE TRUE # true by default
)

find_package(Threads REQUIRED)

target_include_directories(solvent PRIVATE ..)
target_include_directories(solvent PUBLIC ../../include ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(solvent PUBLIC Threads::Threads)
target_link_libraries(solvent PRIVATE solvent_util)