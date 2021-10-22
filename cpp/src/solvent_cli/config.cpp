#include <solvent_cli/repl.hpp>

#include <solvent_cli/config.hpp>
#include <solvent_util/str.hpp>

#include <iostream> // cout, endl,

namespace solvent::cli {

	namespace str = solvent::util::str;
	using namespace solvent::lib;

	using pathkind_t = lib::gen::path::Kind;

	void Config::order(Order new_order) {
		this->order_ = new_order;
	}

	void Config::order(std::string const& new_order_str) {
		std::cout << "\ngrid order is ";
		if (new_order_str.empty()) {
			std::cout << "currently set to: " << order() << std::endl;
			return;
		}
		try {
			const Order new_order = std::stoul(new_order_str);
			switch (new_order) {
			#define SOLVENT_TEMPL_TEMPL(O_) \
				case O_: { order(new_order); std::cout << "now set to: " << order() << std::endl; return; }
			SOLVENT_INSTANTIATE_ORDER_TEMPLATES
			#undef SOLVENT_TEMPL_TEMPL
			}
		} catch (std::invalid_argument const& ia) {
		}
		// unsuccessful return:
		std::cout << order() << " (unchanged).\n"
			<< str::RED.ON << '"' << new_order_str << "\" is not a valid order.\n" << str::RED.OFF
			<< "ORDER OPTIONS: ";
			#define SOLVENT_TEMPL_TEMPL(O_) \
				std::cout << #O_ << ", ";
			SOLVENT_INSTANTIATE_ORDER_TEMPLATES
			#undef SOLVENT_TEMPL_TEMPL
			std::cout << std::endl;
	}


	void Config::verbosity(verbosity::Kind new_output_level) {
		this->verbosity_ = new_output_level;
	}

	void Config::verbosity(std::string const& new_output_level_str) {
		std::cout << "\noutput verbosity is ";
		if (new_output_level_str.empty()) {
			std::cout << "currently set to: " << verbosity() << std::endl;
			return;
		}
		for (unsigned i = 0; i < verbosity::size; i++) {
			if (new_output_level_str.compare(verbosity::NAMES[i]) == 0) {
				verbosity(verbosity::Kind{i});
				std::cout << "now set to: " << verbosity() << std::endl;
				return;
			}
		}
		// unsuccessful return:
		std::cout << verbosity() << " (unchanged).\n"
			<< str::RED.ON << '"' << new_output_level_str << "\" is not a valid verbosity level.\n" << str::RED.OFF
			<< verbosity::OPTIONS_MENU << std::endl;
	}


	void Config::path_kind(const pathkind_t new_path_kind) noexcept {
		path_kind_ = new_path_kind;
	}

	void Config::path_kind(std::string const& new_path_kind_str) noexcept {
		std::cout << "\ngenerator path is ";
		if (new_path_kind_str.empty()) {
			std::cout << "currently set to: " << path_kind() << std::endl;
			return;
		}
		for (unsigned i = 0; i < gen::path::NUM_KINDS; i++) {
			if (new_path_kind_str.compare(gen::path::NAMES[i]) == 0) {
				path_kind(pathkind_t{i});
				std::cout << "now set to: " << path_kind() << std::endl;
				return;
			}
		}
		// unsuccessful return:
		std::cout << path_kind() << " (unchanged).\n"
			<< str::RED.ON << '"' << new_path_kind_str << "\" is not a valid generator path name.\n" << str::RED.OFF
			<< gen::path::OPTIONS_MENU << std::endl;
		return;
	}


	void Config::max_dead_ends(unsigned long long max_dead_ends) {
		this->max_dead_ends_ = max_dead_ends;
	}

	void Config::max_dead_ends(std::string const& new_max_dead_ends_str) {
		std::cout << "\nmax dead ends is ";
		if (new_max_dead_ends_str.empty()) {
			std::cout << "currently set to: " << max_dead_ends()
				<< "\nsetting to zero will default to " << gen::DEFAULT_MAX_DEAD_ENDS(order_)
				<< std::endl;
			return;
		}
		try {
			const unsigned long long new_max_dead_ends = std::stoull(new_max_dead_ends_str);
			max_dead_ends(new_max_dead_ends);
			std::cout << "now set to: " << max_dead_ends() << std::endl;
			// TODO.impl handle negative numbers being parsed as uints
		} catch (std::invalid_argument const& ia) {
			// unsuccessful return:
			std::cout << max_dead_ends() << " (unchanged).\n"
				<< str::RED.ON << '"' << new_max_dead_ends_str << "\" is not a valid uint64_t.\n" << str::RED.OFF
				<< verbosity::OPTIONS_MENU << std::endl;
			return;
		}
	}
}