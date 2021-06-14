#include <solvent_cli/repl.hpp>

#include <solvent_cli/config.hpp>
#include <solvent_util/str.hpp>

#include <iostream> // cout, endl,

namespace solvent::cli {

	namespace str = solvent::util::str;
	using namespace solvent::lib;

	using pathkind_t = lib::gen::path::Kind;

	void Config::set_verbosity(verbosity::Kind new_output_level) {
		this->verbosity_ = new_output_level;
	}


	void Config::set_verbosity(std::string const& new_output_level_str) {
		std::cout << "\noutput level is ";
		if (new_output_level_str.empty()) {
			std::cout << "currently set to: " << get_verbosity() << std::endl;
			return;
		}
		for (unsigned i = 0; i < verbosity::size; i++) {
			if (new_output_level_str.compare(verbosity::NAMES[i]) == 0) {
				set_verbosity(verbosity::Kind{i});
				std::cout << "now set to: " << get_verbosity() << std::endl;
				return;
			}
		}
		// unsuccessful return:
		std::cout << get_verbosity() << " (unchanged).\n"
			<< str::RED.ON << '"' << new_output_level_str << "\" is not a valid verbosity level.\n" << str::RED.OFF
			<< verbosity::OPTIONS_MENU << std::endl;
	}


	void Config::set_path_kind(const pathkind_t new_path_kind) noexcept {
		path_kind_ = new_path_kind;
	}


	void Config::set_path_kind(std::string const& new_path_kind_str) noexcept {
		std::cout << "\ngenerator path is ";
		if (new_path_kind_str.empty()) {
			std::cout << "currently set to: " << get_path_kind() << std::endl;
			return;
		}
		for (unsigned i = 0; i < gen::path::NUM_KINDS; i++) {
			if (new_path_kind_str.compare(gen::path::NAMES[i]) == 0) {
				set_path_kind(pathkind_t{i});
				std::cout << "now set to: " << get_path_kind() << std::endl;
				return;
			}
		}
		// unsuccessful return:
		std::cout << get_path_kind() << " (unchanged).\n"
			<< str::RED.ON << '"' << new_path_kind_str << "\" is not a valid generator path name.\n" << str::RED.OFF
			<< gen::path::OPTIONS_MENU << std::endl;
		return;
	}


	void Config::set_max_dead_ends(unsigned long long max_dead_ends) {
		this->max_dead_ends_ = max_dead_ends;
	}


	void Config::set_max_dead_ends(std::string const& new_max_dead_ends_str) {
		std::cout << "\nmax dead ends is ";
		if (new_max_dead_ends_str.empty()) {
			std::cout << "currently set to: " << get_max_dead_ends() << std::endl;
			return;
		}
		try {
			const unsigned long long new_max_dead_ends = std::stoull(new_max_dead_ends_str);
			set_max_dead_ends(new_max_dead_ends);
			std::cout << "now set to: " << get_max_dead_ends() << std::endl;
			// TODO.impl handle negative numbers being parsed as uints
		} catch (std::invalid_argument const& ia) {
			// unsuccessful return:
			std::cout << get_max_dead_ends() << " (unchanged).\n"
				<< str::RED.ON << '"' << new_max_dead_ends_str << "\" is not a valid uint64_t.\n" << str::RED.OFF
				<< verbosity::OPTIONS_MENU << std::endl;
			return;
		}

	}

}