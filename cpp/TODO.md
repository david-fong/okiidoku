
# Things To Do

## Higher Priority

- https://cmake.org/cmake/help/latest/guide/tutorial/index.html
  - See what can be learned from https://github.com/lefticus/cpp_weekly_game_project
- Decide what interfaces to support:
  - Probably best to start with just readline and a CLI
    - For CLI util libraries, look into using
      - https://github.com/daniele77/cli
      - https://github.com/docopt/docopt.cpp
      - https://github.com/CLIUtils/CLI11
      - http://tclap.sourceforge.net/manual.html
      - https://github.com/Taywee/args
  - Can look into ncurses in the future? Or look into options for TUI libraries?
  - A web interface would be really nice.
- C++20
  - std::bit_width possible usage in size.hpp
  - `using enum`. Might want to wait for CLANG to support?
  - wait for support in gcc and clang for `constexpr std::string`.
    - the ansi constants could use this. Right now, they are stuck as C-style string.
- CLI
  - implement `-h` and `--help` CLI argument.
  - Make giveup thresholds settable via CLI, and allow setting to "default".
  - give a red message when trying to continue and nothing is left to be found.

## Things That Seem To Not Have Worked

These didn't end up doing the thing I wanted / thought might happen.

- Try making traversal order not grid-row-major and see if it improves performance:
  - Hypothesis: Tiles with fewer candidates are like the solution space's dominant bottlenecks. If we were to leave them to the end, we may spend many long advances creating almost-complete solutions that cannot be complete because they violate the bottlenecks (and possibly often in similar ways). If we start with them first, we may be less likely to encounter that problem.
  - Update: I think this didn't work because I was doing naive backtracking. If I improve the backtracking logic. Now I need to remember/find out which traversal paths should make better usage of this and then test it out.

- Use `__builtin_popcount(occmask)`
  - What about if length - popcount is 1?
    - Then I could just set the value right away. This would need a c++ builtin for arbiting a bit.