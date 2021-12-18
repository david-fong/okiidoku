
# Things To Do

## C++ Notes

I didn't want to make a separate file just for a small list of rules / notes to self, and I'm in this file all the time, so I just put it here.

- For defining global variables (not constants!) shared between cpp files, declare prefixed with `extern` in a hpp files, and then define it in one of the cpp files. Functions always have external linkage.
- `inline` means a name can have multiple _identical_ definitions. For defining _and defining_ global constants in headers with a single memory address, prefix the definition with `inline`. Same for functions in headers.

## Higher Priority

1. Do I have any unnecessary usages of the "extern" or "inline" keywords? I should probably make a note to myself to remind when they need to be used.

1. make the grid conversion utilities make use of std::span instead of std::vector? Or just see where spans can be used in general.

1. Consider: The current relabelling canonicalization method may have a big weakness: I think ties can be easily crafted: consider the "Most Canonical" solution grid- it would be all ties. How can this be addressed? (Or perhaps the "Most Canonical" grid is the only weakness?)
    - First of all, how often to ties happen with the current relabelling solution, and what do the grids where this happens look like? Ie. 
    - Break ties by designing a way to give symbols that frequently cohabit atoms label-values that are closer together in value:
    - The cohabitation table can be tiled; a coordinate's vertical or horizontal distance (these are the same, since relabelling moves both the col and row together) from the closest tiling of the main diagonal represents the distance of the labels from each other.
    - For each label, make an array where each index represents another label, and the value is an object containing the left and right distances between them, and the cohabitation count.
    - Hm. So far this seems to suggest a hill-climbing / brute-force-type solution...
    - What if we prioritized labels according to the existing standard deviation information
    - OR... make an empty commit saying that a solution was implemented using the [Ostritch Alorithm](https://en.wikipedia.org/wiki/Ostrich_algorithm)

1. write a scrambler.
1. write some correctness-tests for canonicalization and scrambling.
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

- Can the scramble-invariant property analysis of a grid be used to efficiently estimate the difficulty of a puzzle? I believe (and hope) there is potential that the answer is yes. If it is the case, how? And what would the accuracy of the estimation be (error distribution)?

- Lossless Compression: What specific pattern of cells can always be removed from a solution while guaranteeing that the result is _trivial_ to restore to the full solution?
  - One option:
    - The blocks along a diagonal / shifted diagonal
    - one cell in each remaining block
  - Usefulness:
    - To reduce unnecessary bytes sent over network?
    - Could it be possible to use this as an optimization for solution generation?
      - Set the genpath to skip over these specific cells, and when progress has reached to these cells, switch over to attempting completion.

## Ideas That Seem To Not Have Worked / Are Impractical

These didn't end up doing the thing I wanted / thought might happen.

### Generator

- Try making traversal order not grid-row-major and see if it improves performance:
  - Hypothesis: Cells with fewer candidates are like the solution space's dominant bottlenecks. If we were to leave them to the end, we may spend many long advances creating almost-complete solutions that cannot be complete because they violate the bottlenecks (and possibly often in similar ways). If we start with them first, we may be less likely to encounter that problem.
  - Update: I think this didn't work because I was doing naive backtracking. If I improve the backtracking logic. Now I need to remember/find out which traversal paths should make better usage of this and then test it out.

- Use `popcount(has_mask)`
  - What about if length - popcount is 1?
    - Then I could just set the value right away. This would need a c++ builtin for arbiting a bit.

### Canonicalizer

- See [the bottom of the canonicalization readme](./src/solvent_lib/equiv/readme.md)