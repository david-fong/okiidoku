// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku_cli/config.hpp>

#include <okiidoku_cli_utils/str.hpp>
#include <okiidoku/order.hpp>

#include <iostream> // cout, endl,
#include <charconv> // from_chars
#include <system_error> // errc

namespace okiidoku::cli {

	namespace str = ::okiidoku::util::str;


	void Config::order(const Order new_order) noexcept {
		if (is_order_compiled(new_order)) {
			order_ = new_order;
		}
	}

	void Config::order(const std::string_view arg) {
		if (arg.empty()) {
			std::cout << "is: " << uint_fast32_t{order()} << std::endl;
			return;
		}
		Order new_order {};
		const auto parse_result {std::from_chars(
			arg.data(), arg.data()+arg.size(), new_order
		)};
		if (parse_result.ec == std::errc{} && is_order_compiled(new_order)) {
			order(new_order);
			return;
		}
		std::cout
			<< str::red.on << '"' << arg << "\" is not a valid order.\n" << str::red.off
			<< "ORDER OPTIONS: ";
			#define OKIIDOKU_FOREACH_O_EMIT(O_) \
				std::cout << #O_ << ", ";
			OKIIDOKU_FOREACH_O_DO_EMIT
			#undef OKIIDOKU_FOREACH_O_EMIT
			std::cout << std::endl;
	}


	void Config::canonicalize(bool canonicalize) {
		canonicalize_ = canonicalize;
	}

	void Config::canonicalize(const std::string_view new_canonicalize_str) {
		if (new_canonicalize_str.empty()) {
			std::cout << "is: " << (canonicalize() ? "y" : "n") << std::endl;
			return;
		}
		if (new_canonicalize_str == "y" || new_canonicalize_str == "n") {
			canonicalize(new_canonicalize_str == "y");
		} else {
			std::cout << "is: " << canonicalize() << " (unchanged).\n"
				<< str::red.on << '"' << new_canonicalize_str << "\" does not match `y` or `n`.\n" << str::red.off
				<< std::endl;
			return;
		}
	}
}