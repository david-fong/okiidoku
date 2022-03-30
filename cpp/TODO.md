# Things To Do

## Big Strokes

- compare backtracking and stochastic search statistics
- fix canonicalization for order 3
- minlexing canonicalization
- compare canonicalization methods (performance / time-complexity)
- database
- deductive reasoning + backtracking solver
- puzzle creation experiments (see [bottom section](#interesting-questions-for-further-research))

## Higher Priority

- rebrand: rename project from "solvent" to "ookiidoku"

- previously changed all my includes to use quotes instead of angle brackets after reading [this from Jason Turner's cpp best practices](https://github.com/cpp-best-practices/cppbestpractices/blob/master/03-Style.md#use--for-including-local-files). But here in [cppCoreGuidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#sf12-prefer-the-quoted-form-of-include-for-files-relative-to-the-including-file-and-the-angle-bracket-form-everywhere-else) says to use quotes only for relative path includes. Maybe I misinterpreted/over-interpreted Jason Turner's instructions. Either way, I like the cppCoreGuidelines rationale. Please change back to using it.

- how is vector-of-bool's tweak header thing supposed to work with installation? I don't know how to copy the tweak config to 

- The batch headers use `solvent_util/timer.hpp`. That's not good because the util headers are supposed to be private. Either move timer into include/, or remove the timing capability from batch. I can even imagine a batch api that gives the caller control over when to stop.

- make some grid things for binary and text serdes (and test please).
  - Implement a program in the tools folder to do format conversions.

- using stochastic search:
  - compare intrinsic statistical properties of grids generated (the things currently used in canon\_label).
  - Consider deleting the batching implementation? Or just removing it from the library api (move headers to the src/ folder and don't compile the cpp files; maybe rename the bt/ folder to legacy\_bt/).
    - The backtracking implementation could then just be used as a reference for a more complicated deductive reasoning solver when deductive reasoning can't further progress by deductive reasoning.
      - The path would prioritize the parts of the puzzle that the deductive reasoning knows the least about (the parts that are likely to result in large chain of new possible deductions when filled).

- experiment with the option of making each order be its own dynamic library.
  - Is there anything that would currently make this option wasteful in terms of binary size?
- try making Order an enum
  - see if it can improve switch case cover detection (I think not. I already have some enum-switch-returns that the current gcc warning flags complain about if I don't have a default case).
  - if this works out, make sure to update all the contract docs and remove relevant assertions.
- move the emoji definitions out of the program binary and externalize as a configurable data read in at runtime?

- after experimenting with different stochastic implementations, try implementing an opencl program. The minstd_rand rng is very simple to implement. I think the stochastic algorithm is data-parallelizable.
  - I wonder if there's a bitset implementation for opencl...

- what's this?
  - https://stackoverflow.com/questions/4977252/why-an-unnamed-namespace-is-a-superior-alternative-to-static

- cmake things I can try out:
  - versioning my targets https://cmake.org/cmake/help/latest/prop_tgt/VERSION.html#prop_tgt:VERSION

- Go back and try the old canonicalization by rel row prob, but break ties by doing some brute force: try each tied permutation and valuate it according to some reduction of how it pushes rarer rel counts to the top left. Just be careful to shift to get rid of the main diagonal seam.
  - If there are multiple puddles of ties, the resolution of a puddle shouldn't depend on the resolution of any other puddle- only on the non-tied rows/columns. A consequence of this is that this resolution algorithm will not work if there are no non-tied rows/columns.

1. Consider: The current relabelling canonicalization method may have a big weakness: I think ties can be easily crafted: consider the "Most Canonical" solution grid- it would be all ties. How can this be addressed? (Or perhaps the "Most Canonical" grid is the only weakness?)
    - Break ties by designing a way to give symbols that frequently cohabit atoms label-values that are closer together in value:
    - The cohabitation table can be tiled; a coordinate's vertical or horizontal distance (these are the same, since relabelling moves both the col and row together) from the closest tiling of the main diagonal represents the distance of the labels from each other.
    - For each label, make an array where each index represents another label, and the value is an object containing the left and right distances between them, and the cohabitation count.
    - Hm. So far this seems to suggest a hill-climbing / brute-force-type solution...
    - OR... make an empty commit saying that a solution was implemented using the [Ostrich Algorithm](https://en.wikipedia.org/wiki/Ostrich_algorithm)

- ? Refactor names to use terminology suitable for more than just 2 dimensions? Ex. in 2D: row -> `d0i` (as in "dimension-zero index"), col -> `d1i`. But doing so would imply that I'm going to support multiple dimensions... and that's a huge can of worms.

- (?) Change canonicalization to not use templates? benchmark and compare
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
  - `std::unreachable()`
  - `std::ranges::iota()`
- CLI
  - implement `-h` and `--help` CLI argument.
  - give a red message when trying to continue and nothing is left to be found.

## Interesting Questions for Further Research

- Can the scramble-invariant property analysis of a grid be used to efficiently estimate the difficulty of a puzzle? I believe (and hope) there is potential that the answer is yes. If it is the case, how? And what would the accuracy of the estimation be (error distribution)?
  - If so, could it be used to _create_ probably difficult puzzles?
  - What is the relationship between number of hints and puzzle difficulty?
  - What is the relationship between distribution of hints between houses/chutes and puzzle difficulty?
  - What is the relationship between the relationship between hints and cell relationships and puzzle difficulty?
