// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_ORDER
#define HPP_OKIIDOKU_ORDER

#include <okiidoku/config/defaults.hpp>
#include <okiidoku/detail/order_templates.macros.hpp>
#include <okiidoku/detail/util.hpp>

#include <array>

namespace okiidoku {

	using Order = unsigned char;

	inline constexpr Order largest_compiled_order {[]{
		Order largest {0u};
		#define OKIIDOKU_FOREACH_O_EMIT(O_) largest = (O_);
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		return largest;
	}()};

	[[nodiscard, gnu::const]] constexpr
	bool is_order_compiled(const Order O) noexcept {
		OKIIDOKU_CONTRACT(O <= largest_compiled_order);
		#define OKIIDOKU_FOREACH_O_EMIT(O_) if (O == (O_)) { return true; }
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		return false;
	}

	inline constexpr auto compiled_orders {std::to_array<Order>({
		#define OKIIDOKU_FOREACH_O_EMIT(O_) (O_),
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
	})};
}
#endif