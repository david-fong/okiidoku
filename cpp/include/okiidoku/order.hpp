#ifndef HPP_OKIIDOKU__ORDER
#define HPP_OKIIDOKU__ORDER

#include <okiidoku/okiidoku_config.hpp>
#include <okiidoku_export.h>

#include <bit>

namespace okiidoku {

	using Order = unsigned;

	constexpr unsigned O_MAX = M_OKIIDOKU_O_MAX;
	constexpr unsigned O2_MAX = O_MAX * O_MAX;
	constexpr unsigned O4_MAX = O2_MAX * O2_MAX;

	OKIIDOKU_EXPORT constexpr bool is_order_compiled(Order O) {
		#define M_OKIIDOKU_TEMPL_TEMPL(O_) \
		if (O == O_) return true;
		M_OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef M_OKIIDOKU_TEMPL_TEMPL
		return false;
	}
	static_assert(is_order_compiled(M_OKIIDOKU_DEFAULT_ORDER));

	OKIIDOKU_EXPORT constexpr unsigned get_min_bytes_to_store(const unsigned max_value) {
		return (std::bit_width(max_value) + 7) / 8;
	}
}
#endif