#ifndef HPP_SOLVENT_LIB_GEN_PATH
#define HPP_SOLVENT_LIB_GEN_PATH

#include "../size.hpp"

#include <iostream>
// #include <numeric> // iota
#include <string>
#include <array>

namespace solvent::lib::gen {

	enum class PathDirection : bool {
		Back, Forward,
	};

	enum class ExitStatus {
		Exhausted, Abort, Ok,
	};

	namespace path {
		enum class Kind : unsigned {
			RowMajor,
			DealRwMj,
			BlockCol,
			__MAX__ = BlockCol,
		};
		constexpr size_t NUM_KINDS = static_cast<size_t>(Kind::__MAX__) + 1;
		// Indices of entries must match the
		// literal values_ of their respective enums.
		const std::array<std::string, NUM_KINDS> NAMES = {
			"rowmajor",
			"dealrwmj",
			"blockcol",
		};
		std::ostream& operator<<(std::ostream& os, const Kind path_kind) {
			return os << NAMES[static_cast<unsigned>(path_kind)];
		}
		const std::string OPTIONS_MENU = "\nGEN-PATH OPTIONS:"
			"\n- rowmajor   horizontal strips as wide as the grid one by one"
			"\n- dealrwmj   like dealing cards to each block using row-major"
			"\n- blockcol   rowmajor, but broken into columns one block wide";

		template<solvent::Order O>
		constexpr std::array<typename size<O>::ord4_t (&)(typename size<O>::ord4_t), NUM_KINDS> PathCoords;
	}
}
#endif