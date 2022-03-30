#ifndef HPP_OOKIIDOKU_CLI__ENUM
#define HPP_OOKIIDOKU_CLI__ENUM

#include <iosfwd>
#include <string_view>
#include <array>

namespace ookiidoku::cli {

	namespace verbosity {
		enum class E : unsigned {
			full,
			quiet,
			max_ = quiet,
		};
		constexpr size_t size = static_cast<size_t>(E::max_) + 1;

		// Indices of entries must match the literal values of their respective enums.
		constexpr std::array<std::string_view, size> names {
			"full",
			"quiet",
		};
		inline std::ostream& operator<<(std::ostream& out, const E output_level) {
			return out << names[static_cast<unsigned>(output_level)];
		}
		constexpr std::string_view options_menu_str {
			"\nVERBOSITY OPTIONS:"
			"\n- full    emit all output"
			"\n- quiet   emit summary statistics only"
		};
	}
}
#endif