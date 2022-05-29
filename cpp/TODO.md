# Things To Do

## Roadmap

- deductive reasoning + backtracking solver
  - (add tests and examples)
- draft archive
  - (and update tests and examples)
- refactor and improve canonicalization
- compare backtracking and stochastic search statistics
- implement grid-serdes translator tool
- refine archive
- create a separate github repo for data. add as submodule?
- emscripten build and website
- puzzle creation experiments (see [research-questions.md](../writings/research-questions.md))
- minlexing canonicalization
- compare canonicalization methods (performance / time-complexity)
- language bindings for other languages [see dedicated todo-file](./extras/bindings/todo.md)

## Periodic Checkups

- Examine contents of exported symbols (`nm -nCD libs/okiidoku/.../libokiidoku.so | less`) and make sure nothing is exported that isn't intended to be.
- Ensure dependencies that should be private to the library implementation aren't exposed in any way to the library interface.
- Run aggressive static analyzers
- Check which `#ifdef __EMSCRIPTEN__` blocks can be removed as emscripten updates its sysroot libc++.

## Misc List

- look into cppitertools for writing my raw loops. see if people say anything about performance overhead.

- Am I breaking any of these? https://en.cppreference.com/w/cpp/language/rule_of_three

- boost has a `variant2` class that is never valueless by exception

- find out how to use the [cppcoreguidelines checker](https://docs.microsoft.com/en-us/cpp/code-quality/using-the-cpp-core-guidelines-checkers?view=msvc-170)

- experiment with using compiler "assume" hints (asserts are checked at runtime. assumptions are not checked and used for optimizations (to do the opposite: "remove checks")).
  - [GSL](https://conan.io/center/ms-gsl) has a macro ["`GSL_ASSUME`"](https://github.com/microsoft/GSL/blob/main/include/gsl/assert) to do this on MSVC, Clang, and GCC. Feels weird to add an entire dependency just for a tiny macro though...

- see if grid qualities (like being a solution, being a proper puzzle, being a minimal puzzle), can be encoded through the type system and make it so that always-safe conversions (such as ) are easy, but "unsafe" (not always true) have to either go through an `unsafe_cast_X_grid_to_Y_grid` function, or go through a `checked_cast_X_grid_to_Y_grid`, which may have a non-trivial performance penalty.
  - This would allow making many of the current contracts part of the type system; turning-runtime-error-into-compiler-errors-TM. I would no longer need to write such contract and post-condition comments.
  - Definitely useful types: SolutionGrid, ProperPuzzleGrid, MaybeInvalidPuzzleGrid.
  - Maybe useful types: MinimalPuzzleGrid.
  - Things that might get ugly: the way they are, `grid_is_filled` and `grid_is_valid` would still require the caller to do some kind of unsafe-moving-cast to a new variable of a stronger type.

- try DLL's again? read the docs actually.
  - [general rules and limitations](https://docs.microsoft.com/en-us/cpp/cpp/general-rules-and-limitations)

- profile guided optimization
  - [msvc](https://docs.microsoft.com/en-us/cpp/build/profile-guided-optimizations)
  - [clang](https://clang.llvm.org/docs/UsersManual.html#profile-guided-optimization)

- try writing custom GDB python pretty-printer for `HouseMask`?
  - [how to write a gdb pretty-printer](https://sourceware.org/gdb/onlinedocs/gdb/Writing-a-Pretty_002dPrinter.html#Writing-a-Pretty_002dPrinter)
  - also look into [visual studio `.natvis` files](https://docs.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects)
  - interesting: https://github.com/asarium/gdb-natvis

- [it is allowed in c++ to have template functions and non-template functions with the same name. here are the resulting rules](https://stackoverflow.com/a/16865452/11107541). Could this be used to put the algorithm functions under the same namespace (not in separate "mono" and "visitor" namespace)?

- move the emoji definitions out of the program binary and externalize as a configurable data read in at runtime?

- [what's this? CMakePresets.json](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)

- Go back and try the old canonicalization by rel row prob, but break ties by doing some brute force: try each tied permutation and valuate it according to some reduction of how it pushes rarer rel counts to the top left. Just be careful to shift to get rid of the main diagonal seam.
  - If there are multiple puddles of ties, the resolution of a puddle shouldn't depend on the resolution of any other puddle- only on the non-tied rows/columns. A consequence of this is that this resolution algorithm will not work if there are no non-tied rows/columns.

1. Consider: The current relabelling canonicalization method may have a big weakness: I think ties can be easily crafted: consider the "Most Canonical" solution grid- it would be all ties. How can this be addressed? (Or perhaps the "Most Canonical" grid is the only weakness?)
    - Break ties by designing a way to give symbols that frequently cohabit atoms label-values that are closer together in value:
    - The cohabitation table can be tiled; a coordinate's vertical or horizontal distance (these are the same, since relabelling moves both the col and row together) from the closest tiling of the main diagonal represents the distance of the labels from each other.
    - For each label, make an array where each index represents another label, and the value is an object containing the left and right distances between them, and the cohabitation count.
    - Hm. So far this seems to suggest a hill-climbing / brute-force-type solution...

- Decide what interfaces to support:
  - Probably best to start with just readline and a CLI
    - For CLI util libraries, look into using
      - repl
        - https://github.com/daniele77/cli
      - command
        - https://github.com/CLIUtils/CLI11
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