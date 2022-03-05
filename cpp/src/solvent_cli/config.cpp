#include "solvent_cli/repl.hpp"

#include "solvent_cli/config.hpp"
#include "solvent_util/str.hpp"

#include <iostream> // cout, endl,
#include <charconv> // from_chars

namespace solvent::cli {

	namespace str = solvent::util::str;
	using namespace solvent::lib;

	using pathkind_t = lib::gen::path::Kind;


	void Config::verbosity(verbosity::Kind verbosity) {
		verbosity_ = verbosity;
	}

	void Config::verbosity(const std::string_view new_verbosity_str) {
		if (new_verbosity_str.empty()) {
			std::cout << "is: " << verbosity() << std::endl;
			return;
		}
		for (unsigned i = 0; i < verbosity::size; ++i) {
			if (new_verbosity_str.compare(verbosity::names[i]) == 0) {
				verbosity(verbosity::Kind{i});
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
			#define M_SOLVENT_TEMPL_TEMPL(O_) \
				case O_: { order(new_order); return; }
			M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
			#undef M_SOLVENT_TEMPL_TEMPL
			}
		}
		std::cout
			<< str::red.on << '"' << new_order_str << "\" is not a valid order.\n" << str::red.off
			<< "ORDER OPTIONS: ";
			#define M_SOLVENT_TEMPL_TEMPL(O_) \
				std::cout << #O_ << ", ";
			M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
			#undef M_SOLVENT_TEMPL_TEMPL
			std::cout << std::endl;
	}


	void Config::path_kind(const pathkind_t new_path_kind) noexcept {
		path_kind_ = new_path_kind;
	}

	void Config::path_kind(const std::string_view new_path_kind_str) noexcept {
		if (new_path_kind_str.empty()) {
			std::cout << "is: " << path_kind() << std::endl;
			return;
		}
		for (unsigned i = 0; i < gen::path::num_kinds; ++i) {
			if (new_path_kind_str.compare(gen::path::names[i]) == 0) {
				path_kind(pathkind_t(i));
				return;
			}
		}
		std::cout
			<< str::red.on << '"' << new_path_kind_str << "\" is not a valid generator path name.\n" << str::red.off
			<< gen::path::options_menu_str << std::endl;
		return;
	}


	void Config::max_dead_ends(unsigned long long max_dead_ends) {
		max_dead_ends_ = max_dead_ends;
	}

	void Config::max_dead_ends(const std::string_view new_max_dead_ends_str) {
		if (new_max_dead_ends_str.empty()) {
			std::cout << "is: " << max_dead_ends()
				<< "\nsetting to zero will default to " << gen::cell_dead_ends::limit_default[order_]
				<< "\nvalues above " << gen::cell_dead_ends::limit_i_max[order_] << " will be clamped"
				<< std::endl;
			return;
		}
		unsigned long long new_max_dead_ends {};
		const auto parse_result = std::from_chars(new_max_dead_ends_str.begin(), new_max_dead_ends_str.end(), new_max_dead_ends);
		if (parse_result.ec == std::errc{}) {
			max_dead_ends(new_max_dead_ends);
			// TODO.impl handle negative numbers being parsed as uints
		}
		std::cout << max_dead_ends() << " (unchanged).\n"
			<< str::red.on << '"' << new_max_dead_ends_str << "\" is not a valid value.\n" << str::red.off
			<< verbosity::options_menu_str << std::endl;
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