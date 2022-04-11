#ifndef HPP_OKIIDOKU__ORDER
#define HPP_OKIIDOKU__ORDER

#include <okiidoku/okiidoku_config.hpp>
#include <okiidoku_export.h>

#include <bit>

namespace okiidoku {

	using Order = unsigned;

	OKIIDOKU_EXPORT constexpr bool is_order_compiled(const Order O) {
		#define M_OKIIDOKU_TEMPL_TEMPL(O_) \
		if (O == O_) return true;
		M_OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef M_OKIIDOKU_TEMPL_TEMPL
		return false;
	}

	OKIIDOKU_EXPORT constexpr unsigned get_min_bytes_to_store(const unsigned max_value) {
		return (std::bit_width(max_value) + 7) / 8;
	}
}
#endif