#ifndef HPP_SOLVENT__ORDER
#define HPP_SOLVENT__ORDER

#include "solvent_config.hpp"
#include "solvent_export.h"

#include <cstdint>

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
}
#endif