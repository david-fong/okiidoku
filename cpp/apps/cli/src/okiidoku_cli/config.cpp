// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku_cli/config.hpp>

#include <okiidoku_cli_utils/str.hpp>
#include <okiidoku/order.hpp>

#include <iostream>     // cout
#include <system_error> // errc

namespace okiidoku::cli {

	namespace str = ::okiidoku::util::str;


	void Config::order(const Order order) noexcept {
		if (is_order_compiled(order)) [[likely]] {
			order_ = order;
		}
	}

	void Config::order(const std::string_view arg) {
		if (arg.empty()) { // get/print
			std::cout << "is: " << unsigned{order()} << '\n';
			return;
		}{
			Order new_order {};
			if (str::from_chars(arg, new_order).ec == std::errc{} && is_order_compiled(new_order)) [[likely]] {
				order(new_order);
				return;
			}
		} [[unlikely]]
		std::cout
			<< str::red.on << '"' << arg << "\" is not a valid order.\n" << str::red.off
			<< "order options: "
			#define OKIIDOKU_FOREACH_O_EMIT(O_) \
			<< O_ << ", "
			OKIIDOKU_FOREACH_O_DO_EMIT
			#undef OKIIDOKU_FOREACH_O_EMIT
			<< '\n';
	}


	void Config::canon(bool canon) {
		canon_ = canon;
	}

	void Config::canon(const std::string_view arg) {
		if (arg.empty()) { // get/print
			std::cout << "is: " << (canon() ? "y" : "n") << '\n';
			return;
		}
		if (arg == "y" || arg == "n") {
			canon(arg == "y");
		} else [[unlikely]] {
			std::cout << "is: " << canon() << " (unchanged).\n"
				<< str::red.on << '"' << arg << "\" does not match `y` or `n`.\n" << str::red.off
				<< '\n';
		}
	}
}