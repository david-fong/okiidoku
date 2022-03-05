#ifndef HPP_SOLVENT_CLI__ENUM
#define HPP_SOLVENT_CLI__ENUM

#include <iosfwd>
#include <string_view>
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
		constexpr std::array<std::string_view, size> names {
			"emitall",
			"nogiveups",
			"silent",
		};
		inline std::ostream& operator<<(std::ostream& out, const Kind output_level) {
			return out << names[static_cast<unsigned>(output_level)];
		}
		constexpr std::string_view options_menu_str {
			"\nVERBOSITY OPTIONS:"
			"\n- emitall    emit all output"
			"\n- nogiveups  suppress giveups"
			"\n- silent     emit statistics only"
		};
	}
}
#endif