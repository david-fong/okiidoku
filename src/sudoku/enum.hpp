
#ifndef HPP_SUDOKU_ENUM
#define HPP_SUDOKU_ENUM

#include <iostream>
#include <array>


/**
 * 
 */
namespace Sudoku {

    enum class GiveupMethod {
        OPERATIONS, // Total times attempted to setNextValid.
        BACKTRACKS, // Maximum count searched over all tiles.
        GiveupMethod__MAX = BACKTRACKS, // TODO: alias this with a static field constexpr `size`.
    };
    std::array<std::string, (int)GiveupMethod::GiveupMethod__MAX+1> GiveupMethod_Names = {
        "operations",
        "backtracks",
    }; // TODO: make this accessible from a static getter that takes the scoped enum and static_casts to index.

    enum class GenPath : unsigned {
        ROW_MAJOR,
        BLOCK_COLS,
    };
    // Indices of entries must match the literal values of their respective enums.
    const std::array<std::string, 2> GENPATH_NAMES = {
        "rowmajor",
        "blockcol",
    };
    std::ostream& operator<<(std::ostream& out, const GenPath genPath) {
        return out << GENPATH_NAMES[static_cast<unsigned>(genPath)];
    }

    enum class TvsDirection : bool {
        BACK, FORWARD,
    };
    enum class SolverExitStatus {
        IMPOSSIBLE, GIVEUP, SUCCESS,
    };

    // What to interpret as a blank (non-given) in a puzzle string.
    enum class PuzzleStrBlanksFmt {
        // TODO: implement LENGTH format detector and parser.
        SPACE, ZERO, //LENGTH,
    };


} // End of Sudoku namespace

#endif