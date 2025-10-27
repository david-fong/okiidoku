<!-- SPDX-FileCopyrightText: 2020 David Fong -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
# Morph Notes

## Background

- [sudopedia.enjoysudoku.com](http://sudopedia.enjoysudoku.com/Canonical_Form.html)
- [wikipedia.org](https://wikipedia.org/wiki/Mathematics_of_Sudoku#Essentially_different_solutions)
- [pi.math.cornell.edu](https://pi.math.cornell.edu/~mec/Summer2009/Mahmood/Symmetry.html).
- [handwiki.org](https://handwiki.org/wiki/Mathematics_of_Sudoku#Sudokus_of_other_sizes)

Discussions related to equivalence checking can be found by searching for "Sudoku Canonicalization". Equivalence can be checked by canonicalizing two sudokus and then checking if the results are identical.

- [dobrichev/sudoku-minlexing-tool](https://github.com/dobrichev/sudoku-minlexing-tool)
- [The rust Sudoku library](https://github.com/Emerentius/sudoku/blob/master/src/board/canonicalization.rs)
  - Uses the minlex approach.
- https://www.degruyter.com/document/doi/10.2478/s13537-012-0011-y/pdf
  - Designed for canonicalizing (and then enumerating) _puzzles_- not solutions.
- https://sudokugarden.de/en/info/canonical-form

## Scramble-Invariant Properties of Grids

Scrambling cannot:

- Swap cells between 1xO1 (vertical or horizontal) box-aligned slices (I will call this an "atom").
- Swap cells or atoms between houses.
- Swap boxes between chutes except by swapping entire chutes.
- Change the vertical/horizontal orientation of an atom except by transposing the whole grid.

### Partial Imbalance Analysis

Once source of chute_imbalance is in the distribution of same-atom relations between horizontal or vertical chutes. If such a same-atom relation happens `N` times, the most evenly distributed way for them to occur across one polarity of chute can be represented by creating an array of length `O` containing numbers in `[0, IMBALANCE_MAX]`. `IMBALANCE_MAX` is equal to `2 * (O floor_div 2) * (O - (O floor_div 2))`. Cycle over the entries decrementing a counter from `N` each iteration and incrementing the entry. Imbalance can then be calculated by counting the number of times the relation occurs in each chute, sorting in decreasing order, and then summing the differences between the result and the balanced version.

## My Canonicalization Algorithm

I decided not to try to support direct canonicalization of puzzles. I don't know how much time it would take to adapt the current algorithm to work for that, and there's another way to do it which is much simpler to _understand_: simply solve the puzzle, canonicalize the solution, and apply the same canonicalizing transformations to the puzzle. While it comes with the cost of solving, there is elegance in that this approach is easy to implement outside of any canonicalization algorithm's specific implementation details, and that when the puzzle is solved, the solution will be in canonical form.

### Symbol Canonicalization

Given a grid `G` and a pair of symbols `a` and `b`, for each box `B` of `G`, we can observe whether, and in what orientation `a` and `b` cohabit an atom in `B`.

Given `G`, `a`, and `b`, this could be visualized like so, in an `O1 x O1` grid of `B`s, where "0" represents no atom-cohabitation, "|" represents cohabitation of a vertical atom, and "-" represents cohabitation of a horizontal atom:
```none
- | |
| | 0
0 0 -
```
This is still a positional frame of observation (because of the grid of `B`s). but we want to observe qualities that are position-agnostic.

<!--
\internal I wrote this below sentence in the past and don't understand now what it means or why I wrote it.
each O2*O2 symbol pair (excluding self-pairs) will have such a table, and
when all overlayed together, each position will have O*(O-1)/2 of each of `a`
and `b`, and `O^2 * (O-1)^2` blanks.
-->

Here are some position-agnostic qualities we can extract from this frame of observation:
- How many boxes do the two symbols cohabit an atom in? [0,O2] sort desc.
- Try to normalize "-" / "|".
	- For each cohabitation-orientation cell,
		- How many other cells does it see "pointing to" it?
		- How many other cells does it see "orthogonal to" it?
- Of the atom cohabitations, how many point the same way? [0,O2] sort desc lexicographically of desc sorted per direction
- Of the atom cohabitations, how many

Now that we can assign quantitative measures to relationships between pairs of symbols, we can try to provide an ordering between two symbols based on their relationships with all other symbols.

### Position Canonicalization

This depends on first performing symbol canonicalization.

<!-- TODO -->

### Time Complexity

TODO: update this.

### Result

Note that there are many possible variations on the calculations / measures used in my algorithm that can just as well canonicalize. I made decisions according to the above goals.

The end result of the specific choices made is that:

- Symbols are given such that lower valued ones "play favourites". (they will cohabit atoms with some label values more than others).
- Instances of favouritism will be more concentrated at the top and left of the grid.

This may help with solving- symbol labelling and positioning are "sorted" to specific relationship patterns.

## Other Approaches For Exploration

This was originally more of a "things that I tried and didn't work" section, but I'm rebranding since when I originally tried these, I wasn't taking into account any scramble-invariant properties of grid other than the number of times a pair of label values occurred in the same atom in a grid.

### config_auto_canonicalizer Relabelling

- Linear Algebra Route: Try breaking relabelling ties by using powers of the counts matrix (normalized to the same support). Need to find a continuous version or approximation of the Binomial distribution's PMF. Do some research, and as a last resort, just lerp it or something. Read about the gamma function (continuous version of factorial?). `tgamma(i+1) = i!`
  - A public domain library for the Jacobi algorithm (for diagonalizing dense, real symmetric matrices) https://github.com/jewettaij/jacobi_pd. This could be useful if going through several steps is needed to break ties.
  - This doesn't work for order 2. I know order 2 isn't even marginally interesting, but I think the solution should work for any order, and if it doesn't work for order 2, then something's wrong with it. That being said, I didn't explore this route much. Perhaps with adjustments, it can be useful.

- Graph Theory Route:
  - Could the symbol canonicalization be approached as a travelling salesman problem? I wonder if this would succeed. The cost of each edge would be the probability of that edge's count.
    - See https://www.math.northwestern.edu/documents/book-markov-chains.pdf section 8.2.2.
    - Is NP
  - Graph Bandwidth Problem.
    - Is NP

1. Come up with a weight mapping for each edge in rel_counts and then do a [Floyd-Warshall](https://wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm), and then map each row of the result to either its max or its sum, and then sort. For mapping meanings, see [Closeness Centrality](https://wikipedia.org/wiki/Closeness_centrality)- also the section on disconnected graphs.
    - The weight mapping can be: (recall that `(rel_counts[i][j])/O2` represents the probability in this sudoku grid's markov chain of transitioning from label i to label j.)
      - `O2 - rel_counts[i][j]`
      - `rel_counts[i][j]` interesting? what's would this mean / represent?
      - `p_binom(rel_counts[i][j])` what would this mean / represent?
      - If further differentiation is needed, look into factoring in more scramble-invariant properties.
    - Would this work against the Most Canonical Grid? I don't have high hopes. Will need to test or reason it out.
    - Note that there are other definitions of centrality, such as [Betweenness Centrality](https://wikipedia.org/wiki/Betweenness_centrality), but it uses information that isn't retained in the basic Floyd-Warshall algorithm.
