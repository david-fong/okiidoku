# Things To Do

## Roadmap

- draft database
  - (and update tests and examples)
- test CMake config with different compilers (clang, apple-clang) and try to fix configuration issues
- refactor and improve canonicalization
- compare backtracking and stochastic search statistics
- implement grid-serdes translator tool
- refine database
- create a separate github repo for data. add as submodule?
- deductive reasoning + backtracking solver
  - (add tests and examples)
- emscripten build and website
- puzzle creation experiments (see [research-questions.md](../writings/research-questions.md))
- minlexing canonicalization
- compare canonicalization methods (performance / time-complexity)
- opencl stochastic generator
  - some template/starter code can be found at commit 5a8d4c0 (deleted)
- language bindings for other languages?
  - python, java

## Misc List

- try using clang-tidy
- find out what cppcheck is
- find out what tools can be used to enforce enforceable parts of cppcoreguidelines.
- add sanitizers (address,UB) to cmake debug config.
  - can look at [lefticus weekly game project](https://github.com/lefticus/cpp_weekly_game_project/blob/master/cmake/Sanitizers.cmake) for inspiration.
  - would some bounds-checking assertions become redundant with UB sanitizer?
  - https://docs.microsoft.com/en-us/cpp/sanitizers/asan
- find out how to use the [cppcoreguideline checker](https://docs.microsoft.com/en-us/cpp/code-quality/using-the-cpp-core-guidelines-checkers?view=msvc-170)

- see where ADL can be used to shorten code in a way that isn't mentally taxing. perhaps it should be avoided within the library when it comes to visitor-vs-mono distinction just to be explicit. But maybe for users it can be helpful, since they should only use one of mono or visitor.

- consider supporting CMake 3.22 or 3.21. Ubuntu 20.04's apt repos don't seem to have 3.23. The only 3.23 thing I'm using is `FILE_SET` for target headers

- can the users of the library use [ADL](https://en.cppreference.com/w/cpp/language/adl) to skip specifying the okiidoku namespace for functions consuming Grid?

- try writing custom python pretty-printer for `HouseMask`?
  - [how to write a gdb pretty-printer](https://sourceware.org/gdb/onlinedocs/gdb/Writing-a-Pretty_002dPrinter.html#Writing-a-Pretty_002dPrinter)

- [it is allowed in c++ to have template functions and non-template functions with the same name. here are the resulting rules](https://stackoverflow.com/a/16865452/11107541). Could this be used to put the algorithm functions under the same namespace (not in separate "mono" and "visitor" namespace)?

- Consider a (multi-threaded) batch api that gives the caller control over the logic for when to stop.

- move the emoji definitions out of the program binary and externalize as a configurable data read in at runtime?

- what's this?
  - https://stackoverflow.com/questions/4977252/why-an-unnamed-namespace-is-a-superior-alternative-to-static

- cmake things I can try out:
  - versioning my targets https://cmake.org/cmake/help/latest/prop_tgt/VERSION.html#prop_tgt:VERSION
    - cppcon video <>

- try adding shortcuts to std::views::iota(0,N) and use them instead of all the raw loops. see how it impacts performance. Don't know if compilers will still do all their optimizations (simd, unrolling, etc.).

- Go back and try the old canonicalization by rel row prob, but break ties by doing some brute force: try each tied permutation and valuate it according to some reduction of how it pushes rarer rel counts to the top left. Just be careful to shift to get rid of the main diagonal seam.
  - If there are multiple puddles of ties, the resolution of a puddle shouldn't depend on the resolution of any other puddle- only on the non-tied rows/columns. A consequence of this is that this resolution algorithm will not work if there are no non-tied rows/columns.

1. Consider: The current relabelling canonicalization method may have a big weakness: I think ties can be easily crafted: consider the "Most Canonical" solution grid- it would be all ties. How can this be addressed? (Or perhaps the "Most Canonical" grid is the only weakness?)
    - Break ties by designing a way to give symbols that frequently cohabit atoms label-values that are closer together in value:
    - The cohabitation table can be tiled; a rmiinate's vertical or horizontal distance (these are the same, since relabelling moves both the col and row together) from the closest tiling of the main diagonal represents the distance of the labels from each other.
    - For each label, make an array where each index represents another label, and the value is an object containing the left and right distances between them, and the cohabitation count.
    - Hm. So far this seems to suggest a hill-climbing / brute-force-type solution...
    - OR... make an empty commit saying that a solution was implemented using the [Ostrich Algorithm](https://en.wikipedia.org/wiki/Ostrich_algorithm)

- Decide what interfaces to support:
  - Probably best to start with just readline and a CLI
    - For CLI util libraries, look into using
      - repl
        - https://github.com/daniele77/cli
      - command
        - https://github.com/docopt/docopt.cpp
        - https://github.com/CLIUtils/CLI11
        - http://tclap.sourceforge.net/manual.html
        - https://github.com/Taywee/args
  - Can look into ncurses in the future? Or look into options for TUI libraries?
  - A web interface would be really nice.
- C++20
  - `using enum`. Might want to wait for CLANG to support?
- C++23
  - http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2214r0.html#does-adjacent-mean-2-or-n
  - `std::unreachable()`
  - `std::ranges::iota()`
  - `std::views::chunk()`
    - could this be useful for `canon_place`? I tried messing around with range-v3 on compiler explorer and had trouble with the whole action vs algorithm, container vs view thing. Didn't know what I was doing and could achieve what I wanted.
  - alternative to `std::chunk`, look into `mdspan` (multi-dimensional span). Seems like this is more of what I'm looking for.
  - multidimensional subscript operator
- CLI
  - implement `-h` and `--help` CLI argument.
  - give a red message when trying to continue and nothing is left to be found.
- do I want to generate doxygen docs?
  - https://jothepro.github.io/doxygen-awesome-css/index.html