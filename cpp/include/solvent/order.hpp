#ifndef HPP_SOLVENT__ORDER
#define HPP_SOLVENT__ORDER

#include "solvent/solvent_config.hpp"
#include "solvent_export.h"

#include <bit>

namespace solvent {

	using Order = unsigned;

	constexpr Order O_MAX = M_SOLVENT_O_MAX;
	constexpr Order O2_MAX = O_MAX * O_MAX;
	constexpr Order O4_MAX = O2_MAX * O2_MAX;

	SOLVENT_EXPORT constexpr bool is_order_compiled(Order O) {
		#define M_SOLVENT_TEMPL_TEMPL(O_) \
		if (O == O_) return true;
		M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
		#undef M_SOLVENT_TEMPL_TEMPL
		return false;
	}
	static_assert(is_order_compiled(M_SOLVENT_DEFAULT_ORDER));

	SOLVENT_EXPORT constexpr unsigned get_min_bytes_to_store(const unsigned max_value) {
		return (std::bit_width(max_value) + 7) / 8;
	}
}
#endif