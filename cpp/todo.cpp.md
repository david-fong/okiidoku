
# Things To Do

## Higher Priority

- Design some smarter backtracking:
- CLI
  - implement `-h` and `--help` CLI argument.
  - Make giveup thresholds settable via CLI, and allow setting to "default".
  - give a red message when trying to continue and nothing is left to be found.

## Lower Priority

- Look at ways to use `__builtin_prefetch`.
- Make non-prompt output text print to cout and to outStream (if outStream is not same as cout).
  - This includes the preamble of parameters that were parsed.
  - Investigate if it is simplest to do this with a macro :P.
  - Or just do it explicitly in important places like `Solver::print`, and make them take `isPretty` as an argument.
