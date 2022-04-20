# Things To Do

## Roadmap

- visitor pattern
- see where template deduction guides can make the mono interfaces nicer to use
- test CMake config with different compilers (gcc, clang, apple-clang, MSVC) and try to fix configuration issues
- draft database
- refactor and improve canonicalization
- compare backtracking and stochastic search statistics
- implement grid-serdes translator tool
- refine database
- create a separate github repo for data. add as submodule?
- deductive reasoning + backtracking solver
- emscripten build and website
- puzzle creation experiments (see [bottom section](#interesting-questions-for-further-research))
- minlexing canonicalization
- compare canonicalization methods (performance / time-complexity)
- opencl stochastic generator
  - some template/starter code can be found at commit 5a8d4c0 (deleted)

## Misc List

- now that the batch function is deleted, the mutex for shared_rng is technically not needed for my personal use... hm. I think I should still keep it though...

- change the size-variable versions of the library functions to use the visitor pattern? Need to create a common variant grid class.

- using stochastic search:
  - compare intrinsic statistical properties of grids generated (the things currently used in canon\_label).
  - Consider deleting the backtracking implementation? Or just removing it from the library api (move headers to the src/ folder and don't compile the cpp files; maybe rename the bt/ folder to legacy\_bt/).
    - The backtracking implementation could then just be used as a reference for a more complicated deductive reasoning solver when deductive reasoning can't further progress by deductive reasoning.
      - The path would prioritize the parts of the puzzle that the deductive reasoning knows the least about (the parts that are likely to result in large chain of new possible deductions when filled).

- Consider a batch api that gives the caller control over the logic for when to stop.

- try making Order an enum
  - see if it can improve switch case cover detection (I think not. I already have some enum-switch-returns that the current gcc warning flags complain about if I don't have a default case).
  - if this works out, make sure to update all the contract docs and remove relevant assertions.
- move the emoji definitions out of the program binary and externalize as a configurable data read in at runtime?

- what's this?
  - https://stackoverflow.com/questions/4977252/why-an-unnamed-namespace-is-a-superior-alternative-to-static

- cmake things I can try out:
  - versioning my targets https://cmake.org/cmake/help/latest/prop_tgt/VERSION.html#prop_tgt:VERSION

- try adding shortcuts to std::views::iota(0,N) and use them instead of all the raw loops. see how it impacts performance. Don't know if compilers will still do all their optimizations (simd, unrolling, etc.).

- Go back and try the old canonicalization by rel row prob, but break ties by doing some brute force: try each tied permutation and valuate it according to some reduction of how it pushes rarer rel counts to the top left. Just be careful to shift to get rid of the main diagonal seam.
  - If there are multiple puddles of ties, the resolution of a puddle shouldn't depend on the resolution of any other puddle- only on the non-tied rows/columns. A consequence of this is that this resolution algorithm will not work if there are no non-tied rows/columns.

1. Consider: The current relabelling canonicalization method may have a big weakness: I think ties can be easily crafted: consider the "Most Canonical" solution grid- it would be all ties. How can this be addressed? (Or perhaps the "Most Canonical" grid is the only weakness?)
    - Break ties by designing a way to give symbols that frequently cohabit atoms label-values that are closer together in value:
    - The cohabitation table can be tiled; a coordinate's vertical or horizontal distance (these are the same, since relabelling moves both the col and row together) from the closest tiling of the main diagonal represents the distance of the labels from each other.
    - For each label, make an array where each index represents another label, and the value is an object containing the left and right distances between them, and the cohabitation count.
    - Hm. So far this seems to suggest a hill-climbing / brute-force-type solution...
    - OR... make an empty commit saying that a solution was implemented using the [Ostrich Algorithm](https://en.wikipedia.org/wiki/Ostrich_algorithm)

- ? Refactor names to use terminology suitable for more than just 2 dimensions? Ex. in 2D: row -> `d0i` (as in "dimension-zero index"), col -> `d1i`. But doing so would imply that I'm going to support multiple dimensions... and that's a huge can of worms.

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

## Interesting Questions for Further Research

- Can the scramble-invariant property analysis of a grid be used to efficiently estimate the difficulty of a puzzle? I believe (and hope) there is potential that the answer is yes. If it is the case, how? And what would the accuracy of the estimation be (error distribution)?
  - If so, could it be used to _create_ probably difficult puzzles?
  - What is the relationship between number of hints and puzzle difficulty?
  - What is the relationship between distribution of hints between houses/chutes and puzzle difficulty?
  - What is the relationship between the relationship between hints and cell relationships and puzzle difficulty?

- Over all possible minimal puzzles for a given order size, what is the minimum and maximum number of remaining clues?
  - The minimum number of remaining clues is interesting since it will likely correspond to more difficult puzzles (?)
  - THe maximum number of remaining clues is interesting since it will likely be the worst case for compressed storage.

- Since the stochastic generator cannot deterministically traverse all possible outputs, could it be possible that at a certain point, it becomes easier to find more unique (non-equivalent to previously found) puzzles by continuing a backtracking generator from the output of a stochastic generator's output?

- How does the average number of swaps / swaps-attempts required for the stochastic generator to generate a solution scale with size? what is the distribution of the observed complexity in terms of number of swaps / swap-attempts?