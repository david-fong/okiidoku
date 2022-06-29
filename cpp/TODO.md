# Things To Do

## Roadmap

- improve solver and puzzle-maker for order=5
- draft archive
- refactor and improve canonicalization
- compare backtracking and stochastic search statistics
- implement grid-serdes translator tool
- refine archive
- create a separate github repo for data. add as submodule?
- emscripten build and website
- puzzle creation experiments (see [research-questions.md](../writings/research-questions.md))
- optional async and batch-threading wrappers for long-running things
  - hm. less sure about how necessary this is now knowing about `std::async`.
- minlexing canonicalization
- compare canonicalization methods (performance / time-complexity)
- language bindings for other languages [see dedicated todo-file](./extras/bindings/todo.md)

## Periodic Checkups

- Examine contents of exported symbols (`nm -nCD libs/okiidoku/.../libokiidoku.so | less`) and make sure nothing is exported that isn't intended to be.
- Ensure dependencies that should be private to the library implementation aren't exposed in any way to the library interface.
- Run static analyzers
- Check which `#ifdef __EMSCRIPTEN__` / `__cpp_lib_...` blocks can be removed as emscripten updates its sysroot libc++.
- https://en.cppreference.com/w/cpp/language/rule_of_three

## Misc List

- Come up with a contract-checking strategy for bindings. Options:
  - Leave it as a user responsibility to know and follow contracts.
  - Throw exception on the other language side.

- look into using Catch2 for testing
  - [tutorial](https://github.com/catchorg/Catch2/blob/devel/docs/tutorial.md)
  - [cmake integration](https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md)
- or alternatively using the "smaller version", [doctest](https://github.com/doctest/doctest)

- Consider removing `generate` and instead having a grid constructor that initializes with something that is a valid input for `generate_shuffled`. `iota` is nice and simple, but could also be misused. most-canonical-grid is safer but either is slightly slower (non-constexpr) or consumes constexpr space (constexpr).

- think about [this](https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#Pseudorandom_generators)

- Collect some statistics on how much overlap there usually is between the size-4 UA sets found. A very narrow, simple way to do this is just count how many unique cells are covered, and compare to the number of non-unique cells covered.

- Challenge to self: find out how to make puzzles with few or many givens.
  - Hypothesis: prioritizing to remove cells that are in a house with more givens / fewer candidate-symbols will create puzzles with few givens and vice versa.
  - It should be true that keeping givens in overlapping UA sets should help reduce the number of givens for a minimal puzzle.

- I'd like to collect some statistics about relations between guess stack depth and num puzcells remaining / total num cand-syms remaining.
  - Perhaps there can be some relation to when to search for larger-sized subsets?

- what if we made subset-search also find and apply singles?
  - If specializations for finding singles co-exist with the subset finding, and subsets also find and apply singles, then there should be a documentation note that using the subset finders means the singles finders (presumably?) make no sense to use at the same time.

- what optimizations could be made to quickly check if entire houses have experienced no change? how often would such optimizations come into effect? would there be a net benefit? if so, how much?
- when a subset has been found, we currently always go back to subset-size index 0, but is there anything smarter we could do?
  - If you find a naked subset, the resulting eliminations in the new partition can illuminate new naked subsets, but I don't think it can illuminate new hidden subsets.
  -

- What ways could be explored to speed up the OR-ing together of cand-sym masks when searching for subsets?
  - A tree-like structure where middle layers cache some pre-computed ORs of partial combinations?

- when searching subsets, is there a way to use knowledge about which cells have changed in cand-syms since last subset search to optimize the combination-search?
  - Any combination composed only of cells whose number of cand-syms hasn't changed since the last find attempt can be skipped. (I think).
    - I'd be interested in doing some manual profiling to how often such a skip branch gets taken for order=5.

- try focusing guesses on the cell ruling out a known solution
- see the TODO for `get_guess_grouping`.
- try to use [unavoidable set](https://www.sudopedia.org/wiki/Unavoidable_Set) / deadly pattern detection to optimize proper-puzzle-making.
  - [interesting](http://forum.enjoysudoku.com/max-number-of-clues-t1448.html#p21531).
- possible "deduction technique": given a cell that has multiple candidates, try guessing each candidate (to create "derivatives") and see how much more can be deduced for each derivative without further guessing after that. Any candidates that are ruled out for all the derivatives can be ruled out from the base.
- consider a finder-heuristic that only starts using more powerful finders when the guess stack gets to a certain depth- at which point it unwinds to one-above the last frame that used more powerful finders, and uses more powerful finders.
  - Hm. But the guess stack depth doesn't necessarily correspond to when it may make the most sense to again use more expensive deduction techniques, since I don't know if there's any saying how much further deduction a guess may enable.
  - will have to create some modified version of unwinding which doesn't rule out the guess.
  - or more fine grained: each guess stack frame has a "logic depth/effort" field that describes the finders that were used before the guess was made. Ex. used up to finding subsets of size 2 before the guess was made.
- play around with giving puzzle maker a threshold to stop trying to solve a candidate puzzle after N guesses, or give up (or mark as try-again-later) seeing if a given is safe to remove after N guesses.

- Interesting: [](https://cmake.org/cmake/help/latest/variable/CMAKE_VERIFY_INTERFACE_HEADER_SETS.html)

- cppcoreguidelines C.2: use class if there is an invariant, and struct otherwise.
  - hm. I mean- for a lot of my really simple structs there are invariants on integer bounds. Seems like judgement is required to choose where to draw the line with this guideline.

- look into cppitertools for writing my raw loops. see if people say anything about performance overhead.

- find out how to use the [cppcoreguidelines checker](https://docs.microsoft.com/en-us/cpp/code-quality/using-the-cpp-core-guidelines-checkers?view=msvc-170)
  - I want to use it in a way that is driven by cmake

- see if grid qualities (like being a solution, being a proper puzzle, being a minimal puzzle), can be encoded through the type system and make it so that always-safe conversions (such as ) are easy, but "unsafe" (not always true) have to either go through an `unsafe_cast_X_grid_to_Y_grid` function, or go through a `checked_cast_X_grid_to_Y_grid`, which may have a non-trivial performance penalty.
  - This would allow making many of the current contracts part of the type system; turning-runtime-error-into-compiler-errors-TM. I would no longer need to write such contract and post-condition comments.
  - Definitely useful types: SolutionGrid, ProperPuzzleGrid, MaybeInvalidPuzzleGrid.
  - Maybe useful types: MinimalPuzzleGrid.
  - Things that might get ugly: the way they are, `grid_is_filled` and `grid_is_valid` would still require the caller to do some kind of unsafe-moving-cast to a new variable of a stronger type.

- try DLL's again? read the docs actually.
  - [general rules and limitations](https://docs.microsoft.com/en-us/cpp/cpp/general-rules-and-limitations)

- try writing custom GDB python pretty-printer for `O2BitArr`?
  - [how to write a gdb pretty-printer](https://sourceware.org/gdb/onlinedocs/gdb/Writing-a-Pretty_002dPrinter.html#Writing-a-Pretty_002dPrinter)
  - also look into [visual studio `.natvis` files](https://docs.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects)
  - interesting: https://github.com/asarium/gdb-natvis

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
  - `std::to_underlying()` strange since cppref says all my standard library versions support it already? but it doesn't compile?
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