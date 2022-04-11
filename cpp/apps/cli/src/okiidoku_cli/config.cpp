#include <okiidoku_cli/repl.hpp>

#include <okiidoku_cli/config.hpp>
#include <okiidoku_cli_utils/str.hpp>

#include <iostream> // cout, endl,
#include <charconv> // from_chars

namespace okiidoku::cli {

	namespace str = okiidoku::util::str;


	void Config::verbosity(verbosity::E verbosity) {
		verbosity_ = verbosity;
	}

	void Config::verbosity(const std::string_view new_verbosity_str) {
		if (new_verbosity_str.empty()) {
			std::cout << "is: " << verbosity() << std::endl;
			return;
		}
		for (unsigned i {0}; i < verbosity::size; ++i) {
			if (new_verbosity_str.compare(verbosity::names[i]) == 0) {
				verbosity(verbosity::E{i});
				return;
			}
		}
		std::cout
			<< str::red.on << '"' << new_verbosity_str << "\" is not valid.\n" << str::red.off
			<< verbosity::options_menu_str << std::endl;
	}


	void Config::order(Order new_order) noexcept {
		order_ = new_order;
	}

	void Config::order(const std::string_view new_order_str) {
		if (new_order_str.empty()) {
			std::cout << "is: " << order() << std::endl;
			return;
		}
		int new_order {};
		const auto parse_result = std::from_chars(new_order_str.begin(), new_order_str.end(), new_order);
		if (parse_result.ec == std::errc{}) {
			switch (new_order) {
			#define M_OKIIDOKU_TEMPL_TEMPL(O_) \
				case O_: { order(new_order); return; }
			M_OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
			#undef M_OKIIDOKU_TEMPL_TEMPL
			}
		}
		std::cout
			<< str::red.on << '"' << new_order_str << "\" is not a valid order.\n" << str::red.off
			<< "ORDER OPTIONS: ";
			#define M_OKIIDOKU_TEMPL_TEMPL(O_) \
				std::cout << #O_ << ", ";
			M_OKIIDOKU_INSTANTIATE_ORDER_TEMPLATES
			#undef M_OKIIDOKU_TEMPL_TEMPL
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