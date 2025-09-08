// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__ORDER
#define HPP_OKIIDOKU__ORDER

#include <okiidoku/config/defaults.hpp>
#include <okiidoku/detail/order_templates.macros.hpp>

#include <array>

namespace okiidoku {

	using Order = unsigned char;

	inline constexpr Order largest_compiled_order {[]{
		Order largest {0};
		#define OKIIDOKU_FOREACH_O_EMIT(O_) largest = O_;
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		return largest;
	}()};

	[[nodiscard, gnu::const]] constexpr
	bool is_order_compiled(const Order O) noexcept {
		OKIIDOKU_CONTRACT_USE(O <= largest_compiled_order);
		#define OKIIDOKU_FOREACH_O_EMIT(O_) if (O == O_) { return true; }
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
		return false;
	}

	namespace detail {
		template<Order Ignored, Order... Orders>
		consteval auto CompiledOrdersHelper_make_array() noexcept { return std::to_array({Orders...}); }
	}
	/// \internal exists because my template instantiation macro has no delimiter
	/// argument, so I hack this to ignore a leading comma at a usage site.
	inline constexpr auto compiled_orders {::okiidoku::detail::CompiledOrdersHelper_make_array<
		/* ignored: */Order{0}
		#define OKIIDOKU_FOREACH_O_EMIT(O_) , O_
		OKIIDOKU_FOREACH_O_DO_EMIT
		#undef OKIIDOKU_FOREACH_O_EMIT
	>()};
}
#endif