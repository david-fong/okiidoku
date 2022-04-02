add_library(ookiidoku SHARED
	rng.cpp
	grid.cpp
	db/serdes.cpp
	emoji.cpp
	print_2d.cpp
	morph/transform.cpp
	morph/rel_info.cpp
	morph/canon_label.cpp
	# morph/canon_place.cpp
	morph/canon.cpp
	morph/scramble.cpp
	gen/stochastic.cpp
	gen/batch.cpp
)

# https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
include(GenerateExportHeader)
generate_export_header(ookiidoku)
set_target_properties(ookiidoku PROPERTIES
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN TRUE
)

find_package(Threads REQUIRED)

target_include_directories(ookiidoku
	PRIVATE "${PROJECT_SOURCE_DIR}/src"
	PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# https://cmake.org/cmake/help/latest/command/target_precompile_headers.html
# TODO.low find out why this sometimes/inconsistently errors when combined with `-march=native`
#  possibly maybe related?: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=86007
# target_precompile_headers(ookiidoku
# 	PRIVATE
# 	<iosfwd> <iomanip> <random> <functional> # <chrono>
# 	<algorithm> <compare>
# 	<memory> # <thread> <mutex>
# 	<string> <string_view>
# 	<array> <span> <optional> # <vector>
# 	<bit> <bitset> <cstdint>
# 	<type_traits> <cassert>
# 	# [["ookiidoku/ookiidoku_config.hpp"]]
# 	# [["ookiidoku/order.hpp"]]
# 	# [["ookiidoku/traits.hpp"]]
# 	# [["ookiidoku/grid.hpp"]]
# )

# target_link_libraries(ookiidoku PRIVATE CONAN_PKG::range-v3)
target_link_libraries(ookiidoku PUBLIC Threads::Threads)
target_link_libraries(ookiidoku PRIVATE ookiidoku_util)