# Things To Do

## Higher Priority

- try making Order an enum
  - see if it can improve switch case cover detection.
  - this may also make it possible to statically enforce contracts about orders for the non-template wrapper functions. If so, I will probably neither need to assert that the order is compiled, nor write vacuous default branches for their switch statements.
  - if this works out, make sure to update all the contract docs and remove relevant assertions.
- try benchmarking when has_mask in Generator is changed to use small instead fast definition.
- try benchmarking if Generator member arrays are changed to vectors and see what happens.
- use assertions in the code to get automatic testing of contracts in debug builds.
- consider giving the callback in batch a dedicated mutex, or no mutex at all and leaving it up to the caller. need to consider how likely it is that the bulk of a callback will need synchronization.
- make a custom vector-like class for grids?
  - fixed size and capacity, stores in heap. look into possibility of using boost static_vector.
  - use this as the return type of scramble and canonicalize
- make a buffering RAII adapter for batch callbacks.
- try making the non-template part of batch noinline? It probably won't make a difference, but I just want to try it to learn how to do it.
- in the repl config, consider making some fields per-order. max_dead_ends is a good candidate. verbosity might also be useful, but I'm not sure if it would be surprising in a bad way. should be fine as long as the current values are printed when switching between orders.

- make some grid things for binary serdes.

- "smarter"/greedier backtracking: backtracking may be occurring frequently at a coord because of values much earlier in the genpath progress.
  - backtracking is less likely to occur when other coords in the same house as the stuck coord that have different house types have the same value (overlapping has_mask). Can make an array like a count version of has_mask counting the times a value is taken in each house seen by the stuck coord.
    - Indicators of "bad packing" (lack of overlap): how many more bits are in the coord's has_mask than the has_mask of the house with the most bits in its has_mask?
    - Each coord has a level of prone-ness to bad packing: To get it, walk the gen path, and at each cell, accumulate that cell's coord to a pool, then walk the pool and count how many coords are seen by the current coord. Take `max(count_seen - O2, 0)`.
  - stupid solution: backtrack a distance proportional to the backtrack count of a cell: distance equals `1+floor(log_{base}(backtrack_count))`, where a logical value for `base` is `O2`, since each cell can try up to that many values (distance of backtrack is proportional to the max effort to get back to the backtrack origin), but that is very slow to take effect. If assuming the average probability of going forward at a cell is 0.5, then use `base = O2/2`? stupid because progress does not correlate to coords that are in the same house as the backtrack origin. Can make it so that it only decrements the remaining backtrack distance counter when backtracking from a coord in the same house as the backtrack origin, and then treating an abort as when hitting `progress == 0` with a non-zero remaining backtrack distance counter?
    - The assumption of 0.5 chance of going forward at any coord is not very realistic. A better approximation might take into account a cell's prone-ness to bad packing with the given genpath.

- adapt canonicalization to work on puzzles (incomplete grids).
  - this would allow checking if puzzles are equivalent.

- the canonicalize param of generator seems weird in that the dead_ends grid of the result struct isn't also transformed to match. This feature should either be specced (to also transform dead_ends grid or not) and documented, or removed.
  - I think there's some instruction and data cache optimization opportunity by doing batches of generating and then canonicalization on those batches. In the grand scheme of things, that sounds like over-optimization, but for me, enabling that level of optimization is important. Making it so easy to canonicalize immediately after generating seems like a lazy / somewhat sloppy thing to do from that perspective.

- (maybe?) instead of defining RNGs, make the library functions that use RNG take a reference to an RNG?
  - rationale: give the library user more control over the RNGs. easy for them to seed it, and they can choose whether to share an RNG for gen and shuffle operations.
  - make sure it is still thread safe. Keep the locking mechanism inside the library. don't ask user to provide lock.

- Go back and try the old canonicalization by rel row prob, but break ties by doing some brute force: try each tied permutation and valuate it according to some reduction of how it pushes rarer rel counts to the top left. Just be careful to shift to get rid of the main diagonal seam.
  ```
  count[rel] = 
  exp[rel] = i + j
  scale[rel] = 1 - (p_binomial(count[rel]) ^ 1/O)
  score[rel] = 2 ^ (exp[rel]) * scale[rel]
  find labelling with maximum
    ```

- Forget about canonicalization for now, and focus all efforts on gathering and analysing scramble-invariant properties of grids.

- Try some of [these solutions](https://stackoverflow.com/questions/10897552/call-a-function-before-main) for the RNG default-seeding so that users don't need to manually seed in main unless they want a specific seed. Make sure to test that it works.

1. write some correctness-tests for canonicalization and scrambling.
1. get some benchmarks.

- some of my usages of `extern template` aren't sensible
  - https://arne-mertz.de/2019/02/extern-template-reduce-compile-times/
  - it's only needed when the part _defined_ in the header is big.

1. Consider: The current relabelling canonicalization method may have a big weakness: I think ties can be easily crafted: consider the "Most Canonical" solution grid- it would be all ties. How can this be addressed? (Or perhaps the "Most Canonical" grid is the only weakness?)
    - First of all, how often do ties happen with the current relabelling solution, and what do the grids where this happens look like? Ie. 
    - Break ties by designing a way to give symbols that frequently cohabit atoms label-values that are closer together in value:
    - The cohabitation table can be tiled; a coordinate's vertical or horizontal distance (these are the same, since relabelling moves both the col and row together) from the closest tiling of the main diagonal represents the distance of the labels from each other.
    - For each label, make an array where each index represents another label, and the value is an object containing the left and right distances between them, and the cohabitation count.
    - Hm. So far this seems to suggest a hill-climbing / brute-force-type solution...
    - What if we prioritized labels according to the existing standard deviation information
    - OR... make an empty commit saying that a solution was implemented using the [Ostritch Alorithm](https://en.wikipedia.org/wiki/Ostrich_algorithm)

- make the grid conversion utilities make use of std::span instead of std::vector? Or just see where spans can be used in general.

- some diagnostics to try rendering:
  - A scatter chart showing max-dead-ends vs. num operations. (only caring about success results)
  - a bar graph where each bar counts the number of ResultViews that had a progress (or a furthest coord with a non-zero backtrack count) within the range for that bar's "bin". (to see "how far" aborted generations usually get).
  - comparing the average heatmaps of aborted vs successful generations.
    - See if there are clear differences/patterns in where they usually spike in backtracking. Perhaps this can be used to inform more sophisticated thresholds for each genpath.
- Rename things to match the standard literature / terminology. Do some research to try to get it right. See sudopedia.

- ? Refactor names to use terminology suitable for more than just 2 dimensions? Ex. in 2D: row -> `d0i` (as in "dimension-zero index"), col -> `d1i`. But doing so would imply that I'm going to support multiple dimensions... and that's a huge can of worms.

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
  - `using enum`. Might want to wait for CLANG to support?
  - wait for support in gcc and clang for `constexpr std::string`.
    - the ansi constants could use this. Right now, they are stuck as C-style string.
- I have a nagging feeling that I'm going too far with the namespacing. I'm a C++ noob with no reference so I'm not sure. I can probably make some improvements.
- CLI
  - implement `-h` and `--help` CLI argument.
  - give a red message when trying to continue and nothing is left to be found.

## Interesting Questions for Further Research

- Can the scramble-invariant property analysis of a grid be used to efficiently estimate the difficulty of a puzzle? I believe (and hope) there is potential that the answer is yes. If it is the case, how? And what would the accuracy of the estimation be (error distribution)?

- Lossless Compression: What specific pattern of cells can always be removed from a solution while guaranteeing that the result is _trivial_ to restore to the full solution?
  - One option:
    - The blocks along a diagonal / shifted diagonal
    - one cell in each remaining block?
  - Usefulness:
    - To reduce unnecessary bytes sent over network?
    - Could it be possible to use this as an optimization for solution generation?
      - Set the genpath to skip over these specific cells, and when progress has reached to these cells, switch over to attempting completion.

## Ideas That Seem To Not Have Worked / Are Impractical

These didn't end up doing the thing I wanted / thought might happen.

### Generator

- Using https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size for batch's ThreadFunc generator field. Didn't seem to have any effect. Probably because the generator struct is larger than 64 bytes :P.

- Try making traversal order not grid-row-major and see if it improves performance:
  - Hypothesis: Cells with fewer candidates are like the solution space's dominant bottlenecks. If we were to leave them to the end, we may spend many long advances creating almost-complete solutions that cannot be complete because they violate the bottlenecks (and possibly often in similar ways). If we start with them first, we may be less likely to encounter that problem.
  - Update: I think this didn't work because I was doing naive backtracking. If I improve the backtracking logic. Now I need to remember/find out which traversal paths should make better usage of this and then test it out.
    - hm. but even with skipping backtracks when not-in-same-house as the backtrack origin, there's the projected cost of reaching backtrack origin again. maybe that's why dealrwmj is so bad?

### Canonicalizer

- See [the bottom of the canonicalization readme](./src/solvent_lib/morph/readme.md)
