add_library(solvent_lib SHARED
	# note: gen takes ~20% longer when linked dynamically
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
target_compile_options(solvent_lib
	PRIVATE
	-fvisibility=default
	# ^would like to use default hidden, but that would mean defining macros
	# for supporting many compilers, or writing huge attribute lists. chose
	# default for experimentation and avoiding compiler compat headaches.
)

find_package(Threads REQUIRED)

target_include_directories(solvent_lib PRIVATE ..)
target_include_directories(solvent_lib PUBLIC ../../include)
target_link_libraries(solvent_lib PUBLIC Threads::Threads)
target_link_libraries(solvent_lib PRIVATE solvent_util)