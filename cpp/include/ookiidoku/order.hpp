#ifndef HPP_OOKIIDOKU__ORDER
#define HPP_OOKIIDOKU__ORDER

#include <ookiidoku/ookiidoku_config.hpp>
#include <ookiidoku_export.h>

#include <bit>

namespace ookiidoku {

	using Order = unsigned;

	constexpr Order O_MAX = M_OOKIIDOKU_O_MAX;
	constexpr Order O2_MAX = O_MAX * O_MAX;
	constexpr Order O4_MAX = O2_MAX * O2_MAX;

	OOKIIDOKU_EXPORT constexpr bool is_order_compiled(Order O) {
		#define M_OOKIIDOKU_TEMPL_TEMPL(O_) \
		if (O == O_) return true;
		M_OOKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef M_OOKIIDOKU_TEMPL_TEMPL
		return false;
	}
	static_assert(is_order_compiled(M_OOKIIDOKU_DEFAULT_ORDER));

	OOKIIDOKU_EXPORT constexpr unsigned get_min_bytes_to_store(const unsigned max_value) {
		return (std::bit_width(max_value) + 7) / 8;
	}
}
#endif