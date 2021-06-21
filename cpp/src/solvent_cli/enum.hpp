#ifndef HPP_SOLVENT_CLI_ENUM
#define HPP_SOLVENT_CLI_ENUM

#include <iosfwd>
#include <string>
#include <array>

namespace solvent::cli {

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
		inline const std::array<std::string, size> NAMES = {
			"emitall",
			"nogiveups",
			"silent",
		};
		inline std::ostream& operator<<(std::ostream& out, const Kind output_level) {
			return out << NAMES[static_cast<unsigned>(output_level)];
		}
		inline const std::string OPTIONS_MENU =
			"\nVERBOSITY OPTIONS:"
			"\n- emitall    emit all output"
			"\n- nogiveups  suppress giveups"
			"\n- silent     emit statistics only";
	}
}
#endif