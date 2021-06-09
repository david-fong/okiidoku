
# Things To Do

## Higher Priority

- https://cmake.org/cmake/help/latest/guide/tutorial/index.html
- Take out all stream related things from gen.
  - Read about std::format and see if it has any use for printing.
- Scrap and rewrite the run-multiple info summary reporting.
- Decide what interfaces to support:
  - Probably best to start with just readline and a CLI
    - For CLI util libraries, look into using
      - https://github.com/daniele77/cli
      - https://github.com/docopt/docopt.cpp
      - https://github.com/CLIUtils/CLI11
      - http://tclap.sourceforge.net/manual.html
      - https://github.com/Taywee/args
  - Can look into ncurses in the future?
  - A web interface would be really nice.
- C++20
  - std::bit_width possible usage in size.hpp
  - `typename` doesn't need to be specified as much.
  - `using enum`. Might want to wait for CLANG to support?
  - see what stringy things can now use constexpr strings.
  - wait for support in gcc and clang for `constexpr std::string`.
- CLI
  - implement `-h` and `--help` CLI argument.
  - Make giveup thresholds settable via CLI, and allow setting to "default".
  - give a red message when trying to continue and nothing is left to be found.

## Things That Seem To Not Have Worked

These didn't end up doing the thing I wanted / thought might happen.

- Try making traversal order not grid-row-major and see if it improves performance:
  - Hypothesis: Tiles with fewer candidates are like the solution space's dominant bottlenecks. If we were to leave them to the end, we may spend many long advances creating almost-complete solutions that cannot be complete because they violate the bottlenecks (and possibly often in similar ways). If we start with them first, we may be less likely to encounter that problem.
  - Update: I think this didn't work because I was doing naive backtracking. If I improve the backtracking logic. Now I need to remember/find out which traversal paths should make better usage of this and then test it out.

- Use `__builtin_popcount(occmask)` or `__builtin_ctz(!occmask)` to have special case for when the occmask is full:
  - In that case, just backtrack. Don't go through the search loop because we won't find anything in it.
  - What about if length - popcount is 1?
    - Then I could just set the value right away. This would need a c++ builtin for arbiting a bit.