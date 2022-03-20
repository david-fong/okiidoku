add_library(solvent SHARED
	rng.cpp
	grid.cpp
	db/serdes.cpp
	emoji.cpp
	print_2d.cpp
	morph/canon_label.cpp
	morph/canon_place.cpp
	morph/canon.cpp
	morph/scramble.cpp
	gen/stochastic.cpp
	gen/batch.cpp

	# deprecated:
	# gen/bt/path.cpp
	# gen/bt/generator.cpp
	# gen/bt/batch.cpp
	# gen/bt/batch_print.cpp
)
# https://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html

# https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
include(GenerateExportHeader)
generate_export_header(solvent)
set_target_properties(solvent PROPERTIES
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN TRUE
)

find_package(Threads REQUIRED)

target_include_directories(solvent PRIVATE ..)
target_include_directories(solvent PUBLIC ../../include ${CMAKE_CURRENT_BINARY_DIR})

# https://cmake.org/cmake/help/latest/command/target_precompile_headers.html
# TODO.low find out why this sometimes/inconsistently errors when combined with `-march=native`
#  possibly maybe related?: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=86007
# target_precompile_headers(solvent
# 	PRIVATE
# 	<iosfwd> <iomanip> <random> <functional> # <chrono>
# 	<algorithm> <compare>
# 	<memory> # <thread> <mutex>
# 	<string> <string_view>
# 	<array> <span> <optional> # <vector>
# 	<bit> <bitset> <cstdint>
# 	<type_traits> <cassert>
# 	# [["solvent_config.hpp"]]
# 	# [["solvent/order.hpp"]]
# 	# [["solvent/size.hpp"]]
# 	# [["solvent/grid.hpp"]]
# )

target_link_libraries(solvent PUBLIC Threads::Threads)
target_link_libraries(solvent PRIVATE solvent_util)