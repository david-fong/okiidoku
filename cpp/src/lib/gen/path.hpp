#ifndef HPP_SOLVENT_LIB_GEN_PATH
#define HPP_SOLVENT_LIB_GEN_PATH

#include <array>
#include <string>
#include <iostream>

namespace solvent::lib::gen {

	namespace Path {
		enum class E : unsigned {
			RowMajor,
			DealRwMj,
			BlockCol,
			__MAX__ = BlockCol,
		};
		constexpr size_t size = static_cast<size_t>(E::__MAX__) + 1;
		// Indices of entries must match the
		// literal values of their respective enums.
		const std::array<std::string, size> NAMES = {
			"rowmajor",
			"dealrwmj",
			"blockcol",
		};
		std::ostream& operator<<(std::ostream& out, const E path_kind) {
			return out << NAMES[static_cast<unsigned>(path_kind)];
		}
		const std::string OPTIONS_MENU = "\nGEN-PATH OPTIONS:"
			"\n- rowmajor   horizontal strips as wide as the grid one by one"
			"\n- dealrwmj   like dealing cards to each block using row-major"
			"\n- blockcol   rowmajor, but broken into columns one block wide";
	}

	enum class PathDirection : bool {
		Back, Forward,
	};

	enum class ExitStatus {
		Exhausted, Abort, Ok,
	};
}

#endif