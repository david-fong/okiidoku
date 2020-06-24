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
 * Turning this on allows Solver::traversalOrder to be shared accross
 * Solver instances (ie. a static member). This has huge implications
 * to cache usage. Make sure to turn this off if you write a program
 * that requires coexisting Solvers to have different generator paths.
 */
#define SOLVER_THREADS_SHARE_GENPATH true

#endif
