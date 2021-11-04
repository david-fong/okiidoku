#include <solvent_cli/repl.hpp>

#include <solvent_cli/config.hpp>
#include <solvent_util/str.hpp>

#include <iostream> // cout, endl,

namespace solvent::cli {

	namespace str = solvent::util::str;
	using namespace solvent::lib;

	using pathkind_t = lib::gen::path::Kind;


	void Config::verbosity(verbosity::Kind verbosity) {
		verbosity_ = verbosity;
	}

	void Config::verbosity(std::string const& new_verbosity_str) {
		std::cout << "is ";
		if (new_verbosity_str.empty()) {
			std::cout << "is " << verbosity() << std::endl;
			return;
		}
		for (unsigned i = 0; i < verbosity::size; i++) {
			if (new_verbosity_str.compare(verbosity::NAMES[i]) == 0) {
				verbosity(verbosity::Kind{i});
				return;
			}
		}
		std::cout
			<< str::RED.ON << '"' << new_verbosity_str << "\" is not valid.\n" << str::RED.OFF
			<< verbosity::OPTIONS_MENU << std::endl;
	}


	void Config::order(Order new_order) noexcept {
		order_ = new_order;
	}

	void Config::order(std::string const& new_order_str) {
		if (new_order_str.empty()) {
			std::cout << "is: " << order() << std::endl;
			return;
		}
		try {
			const Order new_order = std::stoul(new_order_str);
			switch (new_order) {
			#define SOLVENT_TEMPL_TEMPL(O_) \
				case O_: { order(new_order); return; }
			SOLVENT_INSTANTIATE_ORDER_TEMPLATES
			#undef SOLVENT_TEMPL_TEMPL
			}
		} catch (std::invalid_argument const& ia) {
		}
		std::cout
			<< str::RED.ON << '"' << new_order_str << "\" is not a valid order.\n" << str::RED.OFF
			<< "ORDER OPTIONS: ";
			#define SOLVENT_TEMPL_TEMPL(O_) \
				std::cout << #O_ << ", ";
			SOLVENT_INSTANTIATE_ORDER_TEMPLATES
			#undef SOLVENT_TEMPL_TEMPL
			std::cout << std::endl;
	}


	void Config::path_kind(const pathkind_t new_path_kind) noexcept {
		path_kind_ = new_path_kind;
	}

	void Config::path_kind(std::string const& new_path_kind_str) noexcept {
		if (new_path_kind_str.empty()) {
			std::cout << "is: " << path_kind() << std::endl;
			return;
		}
		for (unsigned i = 0; i < gen::path::NUM_KINDS; i++) {
			if (new_path_kind_str.compare(gen::path::NAMES[i]) == 0) {
				path_kind(pathkind_t{i});
				return;
			}
		}
		std::cout
			<< str::RED.ON << '"' << new_path_kind_str << "\" is not a valid generator path name.\n" << str::RED.OFF
			<< gen::path::OPTIONS_MENU << std::endl;
		return;
	}


	void Config::max_dead_ends(unsigned long long max_dead_ends) {
		max_dead_ends_ = max_dead_ends;
	}

	void Config::max_dead_ends(std::string const& new_max_dead_ends_str) {
		if (new_max_dead_ends_str.empty()) {
			std::cout << "is: " << max_dead_ends()
				<< "\nsetting to zero will default to " << gen::DEFAULT_MAX_DEAD_ENDS(order_)
				<< std::endl;
			return;
		}
		try {
			max_dead_ends(std::stoull(new_max_dead_ends_str));
			// TODO.impl handle negative numbers being parsed as uints
		} catch (std::invalid_argument const& ia) {
			std::cout << max_dead_ends() << " (unchanged).\n"
				<< str::RED.ON << '"' << new_max_dead_ends_str << "\" is not a valid uint64_t.\n" << str::RED.OFF
				<< verbosity::OPTIONS_MENU << std::endl;
			return;
		}
	}


	void Config::canonicalize(bool canonicalize) {
		canonicalize_ = canonicalize;
	}

	void Config::canonicalize(std::string const& new_canonicalize_str) {
		if (new_canonicalize_str.empty()) {
			std::cout << "is: " << (canonicalize() ? "y" : "n") << std::endl;
			return;
		}
		if (new_canonicalize_str == "y" || new_canonicalize_str == "n") {
			canonicalize(new_canonicalize_str == "y");
		} else {
			std::cout << "is: " << canonicalize() << " (unchanged).\n"
				<< str::RED.ON << '"' << new_canonicalize_str << "\" does not match `y` or `n`.\n" << str::RED.OFF
				<< std::endl;
			return;
		}
	}
}