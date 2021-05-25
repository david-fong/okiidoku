#ifndef HPP_SOLVENT_CLI_ENUM
#define HPP_SOLVENT_CLI_ENUM

#include <iostream>
#include <string>
#include <array>

namespace solvent::cli {

	// TODO.impl Use this setting to change printing behaviour.
	namespace verbosity {
		enum class Kind : unsigned {
			All,
			NoGiveups,
			Silent,
			__MAX__ = Silent,
		};
		constexpr size_t size = static_cast<size_t>(Kind::__MAX__) + 1;
		// Indices of entries must match the
		// literal values of their respective enums.
		const std::array<std::string, size> NAMES = {
			"emitall",
			"nogiveups",
			"silent",
		};
		std::ostream& operator<<(std::ostream& out, const Kind output_level) {
			return out << NAMES[static_cast<unsigned>(output_level)];
		}
		const std::string OPTIONS_MENU =
			"\nOUTPUT-LVL OPTIONS:"
			"\n- emitall    emit all output"
			"\n- nogiveups  suppress giveups"
			"\n- silent     emit statistics only";
	}
}
#endif