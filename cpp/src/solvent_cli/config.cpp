#include "solvent_cli/repl.hpp"

#include "solvent_cli/config.hpp"
#include "solvent_util/str.hpp"

#include <iostream> // cout, endl,

namespace solvent::cli {

	namespace str = solvent::util::str;
	using namespace solvent::lib;

	using pathkind_t = lib::gen::path::Kind;


	void Config::verbosity(verbosity::Kind verbosity) {
		verbosity_ = verbosity;
	}

	void Config::verbosity(const std::string& new_verbosity_str) {
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

	void Config::order(const std::string& new_order_str) {
		if (new_order_str.empty()) {
			std::cout << "is: " << order() << std::endl;
			return;
		}
		try {
			const int new_order = std::stoi(new_order_str);
			switch (new_order) {
			#define M_SOLVENT_TEMPL_TEMPL(O_) \
				case O_: { order(new_order); return; }
			M_SOLVENT_INSTANTIATE_ORDER_TEMPLATES
			#undef M_SOLVENT_TEMPL_TEMPL
			}
		} catch (const std::invalid_argument& ia) {
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

	void Config::path_kind(const std::string& new_path_kind_str) noexcept {
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

	void Config::max_dead_ends(const std::string& new_max_dead_ends_str) {
		if (new_max_dead_ends_str.empty()) {
			std::cout << "is: " << max_dead_ends()
				<< "\nsetting to zero will default to " << gen::cell_dead_ends::limit_default[order_]
				<< "\nvalues above " << gen::cell_dead_ends::limit_i_max[order_] << " will be clamped"
				<< std::endl;
			return;
		}
		try {
			max_dead_ends(std::stoull(new_max_dead_ends_str));
			// TODO.impl handle negative numbers being parsed as uints
		} catch (const std::invalid_argument& ia) {
			std::cout << max_dead_ends() << " (unchanged).\n"
				<< str::red.on << '"' << new_max_dead_ends_str << "\" is not a valid uint64_t.\n" << str::red.off
				<< verbosity::options_menu_str << std::endl;
			return;
		}
	}


	void Config::canonicalize(bool canonicalize) {
		canonicalize_ = canonicalize;
	}

	void Config::canonicalize(const std::string& new_canonicalize_str) {
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