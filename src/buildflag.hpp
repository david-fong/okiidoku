#ifndef HPP_BUILDFLAG
#define HPP_BUILDFLAG

/**
 * 
 * FAST:IO:
 * Note: My implementation does not use C-style IO, so it is safe for
 * consumer code to make the following optimization:
 * ```cpp
 * std::ios_base::sync_with_stdio(false);
 * ```
 */

/**
 * What grid order to use if one is not specified via commandline arg.
 */
#define DEFAULT_ORDER 4

/**
 * Whether or not ansi-escape sequences should be emitted within output
 * to dim out harsh, non-essential text.
 */
#define USE_ANSI_ESC  true

/**
 * Whether or not to count backtrack statistics.
 */
#define BUILDFLAG_CBT true

/**
 * Giveup method.
 */
#define BUILDFLAG_GUM Sudoku::Solver::GUM::E::BACKTRACKS

#endif