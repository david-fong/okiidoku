#ifndef HPP_SUDOKU_SOLVER_ENUM
#define HPP_SUDOKU_SOLVER_ENUM
// This file is a helper for `./solver.hpp`.

#include <iostream>
#include <array>

namespace Sudoku::Solver {

	/**
	 * Give Up Method.
	 *
	 * This is a compile-time argument specified via templating.
	 */
	namespace GUM {
		enum class E : unsigned {
			OPERATIONS, // Total times attempted to setNextValid.
			BACKTRACKS, // Maximum count searched over all tiles.
			GiveupMethod__MAX = BACKTRACKS,
		};
		constexpr size_t size = static_cast<size_t>(E::GiveupMethod__MAX) + 1;
		std::array<std::string, size> NAMES = {
			"operations",
			"backtracks",
		};
		std::ostream& operator<<(std::ostream& out, const E e) {
			return out << NAMES[static_cast<unsigned>(e)];
		}
	}

	namespace GenPath {
		enum class E : unsigned {
			ROW_MAJOR,
			DEAL_RWMJ,
			BLOCK_COL,
			GenPath__MAX = BLOCK_COL,
		};
		constexpr size_t size = static_cast<size_t>(E::GenPath__MAX) + 1;
		// Indices of entries must match the
		// literal values of their respective enums.
		const std::array<std::string, size> NAMES = {
			"rowmajor",
			"dealrwmj",
			"blockcol",
		};
		std::ostream& operator<<(std::ostream& out, const E genPath) {
			return out << NAMES[static_cast<unsigned>(genPath)];
		}
		const std::string OPTIONS_MENU = "\nGEN-PATH OPTIONS:"
			"\n- rowmajor   horizontal strips as wide as the grid one by one"
			"\n- dealrwmj   like dealing cards to each block using row-major"
			"\n- blockcol   rowmajor, but broken into columns one block wide";
	}

	enum class TvsDirection : bool {
		BACK, FORWARD,
	};

	enum class ExitStatus {
		IMPOSSIBLE, GIVEUP, SUCCESS,
	};

} // End of Sudoku::Solver namespace


namespace Sudoku::Repl {

	// TODO.impl Use this setting to change printing behaviour.
	namespace OutputLvl {
		enum class E : unsigned {
			EMIT_ALL,
			SUPPRESS_GIVEUPS,
			SILENT,
			OutputLvl__MAX = SILENT,
		};
		constexpr size_t size = static_cast<size_t>(E::OutputLvl__MAX) + 1;
		// Indices of entries must match the
		// literal values of their respective enums.
		const std::array<std::string, size> NAMES = {
			"emitall",
			"nogiveups",
			"silent",
		};
		std::ostream& operator<<(std::ostream& out, const E outputLvl) {
			return out << NAMES[static_cast<unsigned>(outputLvl)];
		}
		const std::string OPTIONS_MENU =
			"\nOUTPUT-LVL OPTIONS:"
			"\n- emitall    emit all output"
			"\n- nogiveups  suppress giveups"
			"\n- silent     emit statistics only";
	} // End of OutputLvl namespace

} // End of Sudoku::Repl namespace

#endif