
# Things To Do

## Higher Priority

- See if there's a way to use a macro to simplify extern template definitions.
  - One macro for the consuming code to choose sizes to compile,
  - One macro that takes a template and for each of the sizes, substitutes the size into the template by replacing "ORDER".
- try to get rid of the colony include dir.
- mark global variables and function in headers and inline or extern.
- try this: https://arne-mertz.de/2019/02/extern-template-reduce-compile-times/
- Is it necessary to specify `-pthread` in cpp build.sh?
- Design some smarter backtracking.
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
