add_library(solvent_lib STATIC
	grid.cpp
	print.cpp
	# morph/analyze.cpp
	# morph/analyze_label.cpp
	# morph/analyze_place.cpp
	morph/canon.cpp
	morph/scramble.cpp
	gen/path.cpp
	gen/mod.cpp
	gen/batch.cpp
	toolkit.cpp
)

find_package(Threads REQUIRED)

target_include_directories(solvent_lib PRIVATE ..)
target_include_directories(solvent_lib PUBLIC ../../include)
target_link_libraries(solvent_lib PUBLIC Threads::Threads)
target_link_libraries(solvent_lib PRIVATE solvent_util)