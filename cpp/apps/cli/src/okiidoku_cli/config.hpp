// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_CLI_CONFIG
#define HPP_OKIIDOKU_CLI_CONFIG

#include <okiidoku/order.hpp>

#include <string_view>
#include <array>

namespace okiidoku::cli {

	struct Config final {
	public:
		[[nodiscard, gnu::pure]]
		Order order() const noexcept { return order_; }
		// if the specified order is not compiled, no change.
		void order(Order) noexcept;
		void order(std::string_view);

		[[nodiscard, gnu::pure]]
		bool canonicalize() const noexcept { return canonicalize_; }
		void canonicalize(bool);
		void canonicalize(std::string_view);

	private:
		/** invariant: `order_` is always a compiled order. */
		Order order_ {compiled_orders.front()};
		bool canonicalize_ {false};
	};
}
#endif