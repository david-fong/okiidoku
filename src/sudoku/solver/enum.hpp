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
            BLOCK_COLS,
            GenPath__MAX = BLOCK_COLS,
        };
        constexpr size_t size = static_cast<size_t>(E::GenPath__MAX) + 1;
        // Indices of entries must match the
        // literal values of their respective enums.
        const std::array<std::string, size> NAMES = {
            "rowmajor",
            "blockcol",
        };
        std::ostream& operator<<(std::ostream& out, const E genPath) {
            return out << NAMES[static_cast<unsigned>(genPath)];
        }
        const std::string OPTIONS_MENU = "\nGEN-PATH OPTIONS:"
            "\n- rowmajor   horizontal strips as wide as the grid one by one"
            "\n- blockcol   rowmajor, but broken into columns one block wide";
    }

    enum class TvsDirection : bool {
        BACK, FORWARD,
    };

    enum class ExitStatus {
        IMPOSSIBLE, GIVEUP, SUCCESS,
    };

    // What to interpret as a blank (non-given) in a puzzle string.
    enum class PuzzleStrBlanksFmt {
        // TODO [feat] Implement LENGTH format detector and parser.
        SPACE, ZERO, //LENGTH,
    };

} // End of Sudoku::Solver namespace

#endif