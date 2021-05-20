
# Things To Do

## Higher Priority

- [](https://rust-lang.github.io/api-guidelines/interoperability.html)

- Design some smarter backtracking:
  - Problem statement: For higher order grids, "choke-points" are much more apparent. The choke points do not have a strict pattern to where they appear.
  - Goal: Dynamically detect choke-points and implement smart "leaps" backward. Can be like http where each consecutive leap backward from the current choke-point goes further back.
- Make giveup thresholds settable via CLI, and allow setting to "default".
- give a red message when trying to continue and nothing is left to be found.

## Lower Priority

- Look at ways to use `__builtin_prefetch`.
- Make non-prompt output text print to cout and to outStream (if outStream is not same as cout).
  - This includes the preamble of parameters that were parsed.
  - Investigate if it is simplest to do this with a macro :P.
  - Or just do it explicitly in important places like `Solver::print`, and make them take `isPretty` as an argument.
