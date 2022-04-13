add_library(compiled_orders INTERFACE)

# https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
# include(GenerateExportHeader)
# generate_export_header(compiled_orders BASE_NAME okiidoku)
target_sources(compiled_orders INTERFACE FILE_SET HEADERS BASE_DIRS include)


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
# 	# [["okiidoku/compiled_orders.hpp"]]
# 	# [["okiidoku/order.hpp"]]
# 	# [["okiidoku/traits.hpp"]]
# 	# [["okiidoku/grid.hpp"]]
# )