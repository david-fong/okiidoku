
# Things To Do

## Higher Priority

1. make the grid conversion utilities make use of std::span instead of std::vector?

1. Consider: The current relabelling canonicalization method may have a big weakness: I think ties can be easily crafted: consider the "Most Canonical" solution grid- it would be all ties. How can this be addressed? (Or perhaps the "Most Canonical" grid is the only weakness?)
    - Break ties by designing a way to give symbols that frequently cohabit atoms label value that are closer together in value.

1. write a scrambler.
1. write some correctness-tests for canonicalization and scrambling.
    - already found a case where canonicalization seems to be failing (maybe it's a tie-breaking problem?): 709ce82564d31fbaeb4560f7a891d3c2d6284a31fc5b90e7a13fcdb92e0746589f80ae42516dc73b5ac6031db97f284ebde27f864a3c51093417b59c028efa6d47a38ceb95f20d16f2be96043d1875ac0cd9215a76b43e8f8561f7d3e0cab924185d327ecba964f0690bd4af8325ec71c3741b68dfe0a2952efa59c017468bd3 and 708ce92564d31fbaeb4560f7a981d3c2d6294a31fc5b80e7a13fcdb82e0746598f90ae42516dc73b5ac6031db87f294ebde27f964a3c51083417b58c029efa6d47a39ceb85f20d169561f7d3e0cab8240cd8215a76b43e9ff2be86043d1975ac195d327ecba864f0680bd4af9325ec71c3741b69dfe0a2852efa58c017469bd3
1. get some benchmarks and maybe show emerentius.

- some diagnostics to try rendering:
  - A scatter chart showing max-dead-ends vs. num operations
  - a bar graph where each bar counts the number of GenResults that had a frontier_progress within the range for that bar's "bin". (to see "how far" aborted generations usually get). This sounds flawed, since frontier-progress changes frequently during a generation.
  - comparing the average heatmaps of aborted vs successful generations.
    - See if there are clear differences/patterns in where they usually spike in backtracking. Perhaps this can be used to inform more sophisticated thresholds for each genpath.
- Rename things to match the standard literature / terminology. Do some research to try to get it right. See sudopedia.

- (?) Change canonicalization to not use templates. Verify first that it isn't a performance bottleneck.
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
- I have a nagging feeling that I'm going too far with the namespacing. I a C++ noob with no reference so I'm not sure. I can probably make some improvements.
- CLI
  - implement `-h` and `--help` CLI argument.
  - give a red message when trying to continue and nothing is left to be found.

## Interesting Questions for Further Research

- Is there any correlation between the potential for creating difficult puzzles from a solution and the value-pair-same-atom-count-probability table I'm using for canonicalization? Or perhaps the specific cells that are removed (targeting and those relationships in specific ways)?

- What specific pattern of cells can always be removed from a solution while guaranteeing that the result remains possible and easy to restore to the full solution?
  - One option:
    - The blocks along a diagonal / shifted diagonal
    - one cell in each remaining block
  - Usefulness:
    - To reduce unnecessary bytes sent over network?
    - Could it be possible to use this as an optimization for solution generation?
      - Set the genpath to skip over these specific cells, and when progress has reached to these cells, switch over to attempting completion.
    - Notice that this allows putting an intuitive upper bound on the known number of possible solutions for a given grid order (`(O2!)^(O1*(O1-1))`). But other people have probably come up with better calculations; will need to do some reading.

## Things That Seem To Not Have Worked

These didn't end up doing the thing I wanted / thought might happen.

- Try making traversal order not grid-row-major and see if it improves performance:
  - Hypothesis: Cells with fewer candidates are like the solution space's dominant bottlenecks. If we were to leave them to the end, we may spend many long advances creating almost-complete solutions that cannot be complete because they violate the bottlenecks (and possibly often in similar ways). If we start with them first, we may be less likely to encounter that problem.
  - Update: I think this didn't work because I was doing naive backtracking. If I improve the backtracking logic. Now I need to remember/find out which traversal paths should make better usage of this and then test it out.

- Use `popcount(has_mask)`
  - What about if length - popcount is 1?
    - Then I could just set the value right away. This would need a c++ builtin for arbiting a bit.