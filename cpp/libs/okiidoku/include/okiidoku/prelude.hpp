#ifndef HPP_OKIIDOKU__PRELUDE
#define HPP_OKIIDOKU__PRELUDE

#include <okiidoku/config/defaults.hpp>
#include <okiidoku_export.h>

namespace okiidoku {

	using Order = unsigned;

	consteval bool is_order_compiled(const Order O) {
		#define OKIIDOKU_FOR_COMPILED_O(O_) \
		if (O == O_) return true;
		OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
		#undef OKIIDOKU_FOR_COMPILED_O
		return false;
	}
}
#endif