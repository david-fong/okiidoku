
# Things To Do

## Higher Priority

1. Come up with a weight mapping for each edge in rel_counts and then do a [Floyd-Warshall](https://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm), and then map each row of the result to either its max or its sum, and then sort. For mapping meanings, see [Closeness Centrality](https://en.wikipedia.org/wiki/Closeness_centrality)- also the section on disconnected graphs.
    - The weight mapping can be: (recall that `(rel_counts[i][j])/O2` represents the probability in this sudoku grid's markov chain of transitioning from label i to label j.)
      - `O2 - rel_counts[i][j]`
      - `rel_counts[i][j]` interesting? what's would this mean / represent?
      - `p_binom(rel_counts[i][j])` what would this mean / represent?
      - If further differentiation is needed, look into factoring in  information about:
        - blocks being in the same chute: retain in rel_counts, instead of a count, store a mask where each bit contains whether that block has an atom with this relationship. The count is obtained by counting 1-bits, and each 1-bit can have a weight according to the number of blocks in same chutes that also have this bit on.
        - The "skew"-neww of the atom relationships: what is the ratio of contributing horizontal vs vertical atoms?
    - Would this work against the Most Canonical Grid? I don't have high hopes. Will need to test or reason it out. Wait a second- the relabelling doesn't need to handle the MCG right? Because it would be handled afterward by the repositioning (I think?)!
    - Note that there are other definitions of centrality, such as [Betweenness Centrality](https://en.wikipedia.org/wiki/Betweenness_centrality), but it uses information that isn't retained in the basic Floyd-Warshall algorithm

1. make the grid conversion utilities make use of std::span instead of std::vector? Or just see where spans can be used in general.

1. Consider: The current relabelling canonicalization method may have a big weakness: I think ties can be easily crafted: consider the "Most Canonical" solution grid- it would be all ties. How can this be addressed? (Or perhaps the "Most Canonical" grid is the only weakness?)
    - First of all, how often to ties happen with the current relabelling solution, and what do the grids where this happens look like? Ie. 
    - Break ties by designing a way to give symbols that frequently cohabit atoms label-values that are closer together in value:
    - In the cohabitation table, the table tiles, and a coordinate's vertical or horizontal distance (since relabelling moves both the col and row together, these are the same) from the closest tiling of the main diagonal represents the distance of the labels from each other.
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

## Ideas That Seem To Not Have Worked / Are Impractical

These didn't end up doing the thing I wanted / thought might happen.

### Generator

- Try making traversal order not grid-row-major and see if it improves performance:
  - Hypothesis: Cells with fewer candidates are like the solution space's dominant bottlenecks. If we were to leave them to the end, we may spend many long advances creating almost-complete solutions that cannot be complete because they violate the bottlenecks (and possibly often in similar ways). If we start with them first, we may be less likely to encounter that problem.
  - Update: I think this didn't work because I was doing naive backtracking. If I improve the backtracking logic. Now I need to remember/find out which traversal paths should make better usage of this and then test it out.

- Use `popcount(has_mask)`
  - What about if length - popcount is 1?
    - Then I could just set the value right away. This would need a c++ builtin for arbiting a bit.

### Canonicalizer Relabelling

- Linear Algebra Route: Try breaking relabelling ties by using powers of the counts matrix (normalized to the same support). Need to find a continuous version or approximation of the Binomial distribution's PMF. Do some research, and as a last resort, just lerp it or something. Read about the gamma function (continuous version of factorial?). `tgamma(i+1) = i!`
  - A public domain library for the Jacobi algorithm (for diagonalizing dense, real symmetric matrices) https://github.com/jewettaij/jacobi_pd. This could be useful if going through several steps is needed to break ties.
  - This doesn't work for order 2. I know order 2 isn't even marginally interesting, but I think the solution should work for any order, and if it doesn't work for order 2, then something's wrong with it. That being said, I didn't explore this route much. Perhaps with adjustments, it can be useful.

- Graph Theory Route: 
  - Could the relabelling canonicalization be approached as a travelling salesman problem? I wonder if this would succeed. The cost of each edge would be the probability of that edge's count.
    - See https://www.math.northwestern.edu/documents/book-markov-chains.pdf section 8.2.2.
    - Is NP
  - Graph Bandwidth Problem.
    - Is NP