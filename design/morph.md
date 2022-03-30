# Morph Notes

## Background

Two Sudokus are equivalent if some combination of validity-preserving transformations which do not change the interrelationships of labels in the Sudokus can cause them to become identical.

Such transformations include:

- shuffling rows/columns within row/column groups
- shuffling row/column groups
- mirroring along a diagonal
- and shuffling labels (ie. numbers. ex. replacing all ones with twos and vice versa)
- Note that rotation is included implicitly, since it can be performed by combinations of the other transformations.

Other people have also written about this topic, but focusing mainly on _counting_ equivalent and non-equivalent Sudokus. I didn't read any before drafting the following parts of this writing, so the terminology I use later is not the same as the community standard.

- [wikipedia.org](https://en.wikipedia.org/wiki/Mathematics_of_Sudoku#Essentially_different_solutions)
- [pi.math.cornell.edu](https://pi.math.cornell.edu/~mec/Summer2009/Mahmood/Symmetry.html).
- [handwiki.org](https://handwiki.org/wiki/Mathematics_of_Sudoku#Sudokus_of_other_sizes)

There isn't an abundance of community discussion on the topic of equivalence checking:

- [this math stack exchange post](https://math.stackexchange.com/questions/1903186/determining-if-two-sudoku-boards-are-in-the-same-equivalence-class/4135784#4135784)
- [this math stack exchange post](https://math.stackexchange.com/questions/2791524/group-theory-and-sudoko)

Discussions related to equivalence checking can be found by searching for "Sudoku Canonicalization". Equivalence can be checked by canonicalizing two sudokus and then checking if the results are identical.

- [sudopedia.enjoysudoku.com](http://sudopedia.enjoysudoku.com/Canonical_Form.html)
- [dobrichev/sudoku-minlexing-tool](https://github.com/dobrichev/sudoku-minlexing-tool)
- [The rust Sudoku library](https://github.com/Emerentius/sudoku/blob/master/src/board/canonicalization.rs)
  - Uses the minlex approach.
- https://www.degruyter.com/document/doi/10.2478/s13537-012-0011-y/pdf
  - Designed for canonicalizing (and then enumerating) _puzzles_- not solutions.
- https://sudokugarden.de/en/info/canonical-form

## Scramble-Invariant Properties of Grids

Scrambling cannot:

- Swap cells between 1xO1 (vertical or horizontal) block-aligned slices (I will call this an "atom").
- Swap cells or atoms between houses.
- Swap blocks between chutes.
- Change the vertical/horizontal orientation of an atom without doing so for all atoms (transposing the grid).

### A Bounty of Information

I was surprised to find so many.

- I am only looking at size-2 relationships between labels, but one can observe and use the nature and number of relationships of size up to the grid order (the size of an atom). I only look at size-2 because I'm not sure how to gather this information and retain the minimum necessary amount so that space usage doesn't get out of hand, since the size of the observed relationships determines of the number of dimensions of the multidimensional array, with each dimension having length `o^2`.
  - Some observations:
    - A grid has `2 * o^3` atoms. Analysis can be performed with relationships within atoms of sizes in `[2, o]`. For a size `s`, there are `nCr(o^2, s)` possible atoms.
    - As the size of the grid increases, the 

- For canonicalization of labelling, I am only looking at the counts of relationships between labels- which only makes use of the first "scrambling-cannot" bullet. The combination of all the bullets would be, for each relationship between two labels, to gather a trinary "mask", where each digit corresponds to one of the blocks, and the value of the digit is either "no relationship", "horizontal atom relationship", or "vertical atom relationship". This mask cannot be used raw, since the repositioning of chutes will reorder digits, and transposition of the grid will swap the orientation of atoms. The mask itself must be canonicalized in terms of placement (unless placement canonicalization has already been performed, but I'm not sure if that's possible to achieve). I am currently using the horizontal and vertical masks OR-ed together.

### Partial Imbalance Analysis

Once source of chute_imbalance is in the distribution of same-atom relations between horizontal or vertical chutes. If such a same-atom relation happens `N` times, the most evenly distributed way for them to occur across one polarity of chute can be represented by creating an array of length `O` containing numbers in `[0, IMBALANCE_MAX]`. `IMBALANCE_MAX` is equal to `2 * (O floor_div 2) * (O - (O floor_div 2))`. Cycle over the entries decrementing a counter from `N` each iteration and incrementing the entry. Imbalance can then be calculated by counting the number of times the relation occurs in each chute, sorting in decreasing order, and then summing the differences between the result and the balanced version.

## My Canonicalization Algorithm

I decided not to try to support direct canonicalization of puzzles. I don't know how much time it would take to adapt the current algorithm to work for that, and there's another way to do it which is much simpler to _understand_: simply solve the puzzle, canonicalize the solution, and apply the same canonicalizing transformations to the puzzle. While it comes with the cost of solving, there is elegance in that this approach is easy to implement outside of any canonicalization algorithm's specific implementation details.

### High Level Approach

- base the canonicalization on comparing measures of chute_imbalance in the grid's scramble-invariant properties.

- default to factoring as many cells as possible into the sorting bases, and only factoring them out when necessary to break ties instead of vice versa.

### Time Complexity

If anyone would like to verify my evaluation here it would be much appreciated. Take a look at [`canon_label.cpp`](./canon_label.cpp) and [`canon_place.cpp`](./canon_place.cpp).

- label
  - getting rel mask table: iterate atoms (`O3`). for each atom, iterate pairs of labels (`nCr(O1,2) = O1!/(2*(O1-2)!)`). total: `O3 * O1!/(2*(O1-2)!)`.
  - iterate rel mask table (`O4`). for each rel mask, get polar counts (`O1`), and sort them (`O1*log(O1)`). total: `O4 * O1*log(O1)`.
  - iterate placeless info table (`O4`) and if polarities should be swapped, do it (`O4`). total: `O4`.
  - sort rows of placeless info table (`O2 * O2*log(O2)`).
  - reduce rows of placeless info table (`O2 * O2`).
  - optional: transpose placeless info table for a possible cache optimization. I'm not sure if this is actually effective. needs benchmarking. (`O4`).
  - sort the placeless info table lexicographically. I don't know what the time complexity for this is. definitely less than `O2*log(O2) * O2` (if every lexicographical compare gets worst-case tie-breaking).

Time complexity isn't everything. There's also space usage and caching. I've tried to carefully take those into account as well.

### Result

Note that there are many possible variations on the calculations / measures used in my algorithm that can just as well canonicalize. I made decisions according to the above goals.

The end result of the specific choices made is that:

- Labels are given such that lower valued labels "play favourites". (they will cohabit atoms with some label values more than others).
- Instances of favouritism will be more concentrated at the top and left of the grid.

This may help with solving- labelling and positioning are "sorted" to specific relationship patterns.

## Other Approaches For Exploration

This was originally more of a "things that I tried and didn't work" section, but I'm rebranding since when I originally tried these, I wasn't taking into account any scramble-invariant properties of grid other than the number of times a pair of label values occurred in the same atom in a grid.

### config_auto_canonicalizer Relabelling

- Linear Algebra Route: Try breaking relabelling ties by using powers of the counts matrix (normalized to the same support). Need to find a continuous version or approximation of the Binomial distribution's PMF. Do some research, and as a last resort, just lerp it or something. Read about the gamma function (continuous version of factorial?). `tgamma(i+1) = i!`
  - A public domain library for the Jacobi algorithm (for diagonalizing dense, real symmetric matrices) https://github.com/jewettaij/jacobi_pd. This could be useful if going through several steps is needed to break ties.
  - This doesn't work for order 2. I know order 2 isn't even marginally interesting, but I think the solution should work for any order, and if it doesn't work for order 2, then something's wrong with it. That being said, I didn't explore this route much. Perhaps with adjustments, it can be useful.

- Graph Theory Route: 
  - Could the relabelling canonicalization be approached as a travelling salesman problem? I wonder if this would succeed. The cost of each edge would be the probability of that edge's count.
    - See https://www.math.northwestern.edu/documents/book-markov-chains.pdf section 8.2.2.
    - Is NP
  - Graph Bandwidth Problem.
    - Is NP

1. Come up with a weight mapping for each edge in rel_counts and then do a [Floyd-Warshall](https://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm), and then map each row of the result to either its max or its sum, and then sort. For mapping meanings, see [Closeness Centrality](https://en.wikipedia.org/wiki/Closeness_centrality)- also the section on disconnected graphs.
    - The weight mapping can be: (recall that `(rel_counts[i][j])/O2` represents the probability in this sudoku grid's markov chain of transitioning from label i to label j.)
      - `O2 - rel_counts[i][j]`
      - `rel_counts[i][j]` interesting? what's would this mean / represent?
      - `p_binom(rel_counts[i][j])` what would this mean / represent?
      - If further differentiation is needed, look into factoring in more scramble-invariant properties.
    - Would this work against the Most Canonical Grid? I don't have high hopes. Will need to test or reason it out.
    - Note that there are other definitions of centrality, such as [Betweenness Centrality](https://en.wikipedia.org/wiki/Betweenness_centrality), but it uses information that isn't retained in the basic Floyd-Warshall algorithm.
