add_library(okiidoku_base INTERFACE)

add_custom_target(okiidoku_export_header)
# https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
include(GenerateExportHeader)
generate_export_header(okiidoku_export_header BASE_NAME okiidoku)

target_sources(okiidoku_base PUBLIC FILE_SET HEADERS BASE_DIRS include)
target_include_directories(okiidoku_base
	# PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	# $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)



# https://cmake.org/cmake/help/latest/command/target_precompile_headers.html
# TODO.low find out why this sometimes/inconsistently errors when combined with `-march=native`
#  possibly maybe related?: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=86007
# target_precompile_headers(okiidoku
# 	PRIVATE
# 	<iosfwd> <iomanip> <random> <functional> # <chrono>
# 	<algorithm> <compare>
# 	<memory> # <thread> <mutex>
# 	<string> <string_view>
# 	<array> <span> <optional> # <vector>
# 	<bit> <bitset> <cstdint>
# 	<type_traits> <cassert>
# 	# [["okiidoku/okiidoku_config.hpp"]]
# 	# [["okiidoku/order.hpp"]]
# 	# [["okiidoku/traits.hpp"]]
# 	# [["okiidoku/grid.hpp"]]
# )