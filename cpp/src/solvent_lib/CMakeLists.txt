add_library(solvent_lib STATIC
	grid.cpp
	print.cpp
	# equiv/analyze.cpp
	equiv/canon.cpp
	equiv/scramble.cpp
	gen/path.cpp
	gen/mod.cpp
	gen/batch.cpp
	toolkit.cpp
)
target_include_directories(solvent_lib PRIVATE ..)
target_include_directories(solvent_lib PUBLIC ../../include)
target_link_libraries(solvent_lib PRIVATE solvent_util)