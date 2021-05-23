
# Things To Do

## Higher Priority

- Take out all stream related things from gen.
- Move things from gen to grid.
- Make genpath non-settable.
  - Either make it a constexpr function set intenally at construction,
  - OR Make the generate function templated on it.
- Decide what interfaces to support:
  - ncurses? readline? web? plain std::getline?
- Design some smarter backtracking.
- Scrap and rewrite the run-multiple info printing.
- Move the run-multiple business logic to the lib folder.
- C++20
  - `typename` doesn't need to be specified as much.
  - `using enum`. Might want to wait for CLANG to support?
  - Use `<bit>` to get ctlz and such.
  - see what stringy things can now use constexpr strings.
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
