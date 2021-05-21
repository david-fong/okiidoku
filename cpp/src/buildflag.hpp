#ifndef HPP_BUILDFLAG
#define HPP_BUILDFLAG

/**
 * Whether or not ansi-escape sequences should be emitted within output
 * to dim out harsh, non-essential text.
 */
#define USE_ANSI_ESC true

/**
 * Whether or not to enable windows ansi escape code handling.
 */
#define WINDOWS_ANSI (USE_ANSI_ESC && false)

/**
 * Turning this on allows Solver::traversalOrder to be shared accross
 * Solver instances (ie. a static member). This has huge implications
 * to cache usage. Make sure to turn this off if you write a program
 * that requires coexisting Solvers to have different generator paths.
 */
#define SOLVER_THREADS_SHARE_GENPATH false

#endif
