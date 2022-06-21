# Puzzles

## Puzzle Generation

Goals: Generate minimal, difficult puzzles. TODO: by what definition of difficulty?

## Solver

[sudopedia - solving techniques](https://www.sudopedia.org/wiki/Solving_Technique)

There are two paths here: one is to create a solver that purely optimizes for speed/throughput ("fast solver"), and the other is to prioritize using human solving strategies and be able to rank the difficulty of the puzzle based on the difficulty of techniques used ("reasoning solver").

Another design decision: should the solver be able to detect when there are multiple solutions and no solutions? Or should it assume proper puzzles (puzzles with exactly one solution)?

I want to create a solver that can detect when there are no solutions and walk all possible solutions.

My first priority is to get a solver that purely optimizes for speed/throughput. I may try to create a "sudoku explainer" (puzzle difficulty rater) in the future.

### Cell-Major Pencilmark Representation

Note: I don't know if there is official terminology. I came up with the term "cell-major" independently. If there is official terminology, I'd prefer to use it.

Cell-major pencilmark representation arranges (in memory) the pencilmark data structure with cells in the outer dimension, and candidate-symbols in the inner dimension. Using such a representation, it is operationally trivial to detect when a cell only has one candidate-symbol, but less so to detect when a symbol only has one candidate-cell in any of its host houses.

This is as opposed to symbol-major representation, which arranges symbols in the outer dimension, and candidate-cells in the inner dimension.

Cell-major representation came to my mind first because when playing sudoku, it is easy to write candidate symbols inside the cells of a grid. Visualizing single-digit candidate-cells is easy to do with supporting software, but not as easy on paper.

Symbol-major representation has a slight _dis_-advantage in that its singles method requires scanning its candidate-cells in each of its (_three!_) host houses, and (especially considering we want to support variable-size grids,) a CPU doesn't have extremely fast custom instructions to scan the column and box selection masks assuming that the bits for cells are laid out in row-major order. Compare this to cell-major order, whose singles method checks a single O2-length, contiguous bit-array of candidate-symbols and can use popcount instructions, with the bit-array fitting within one 64-bit word for orders below 9.

Symbol-major does have an advantage in terms of spatial-locality of operation when it comes to eliminating candidate-cells in the three houses hosting a cell in-lieu of committing a symbol to it: it has a _per-symbol_ O4-length bit-array, whereas the the cell-major grid can comparatively be described as having a _symbol-interleaved_ candidate-cell bit-array. This would make it possible to optimize updating the row house, and have some (but less) optimization to updating the box house, but probably no optimization to updating the column house (again, assuming row-major layout of candidate-cells).

At this moment, I have no idea which one is actually better in practice. The above is just analysis.

### Candidate Elimination Independencies

- After CellClaimSym at A, can check
  - SymClaimCell: all houses neighbouring A for any symbols that were removed from A.

- After SymClaimCell at A, can check:
  - CellClaimSym: n/a
  - LockedCands: box of A to see if any symbols removed from A can now go only in one line intersecting A.

## Puzzle Making

How is it done?

- Perhaps by searching for the next given to remove that retains the most symbol and cell candidates?

- Perhaps there is some correlation to the properties currently being used for canonicalizing labels?

hypothesis: if removing a given breaks properness, the only way to change that is by adding back other removed givens ("backtracking" of removing a given). If this is true, once a given is known to break properness if removed, if no backtracking is done ever, then it can never be removed.

hypothesis: making guesses all packed in the same / nearby line-box intersections increases the probability of finding "contradictory" guesses earlier, since those guesses will concentrate their eliminations in the same lines and boxes.

hypothesis: given a cell, C, the best guess of a symbol is the symbol, S, who has the most other candidate-cells in the same house as C. Guessing S will eliminate S as a candidate-sym from more cells, and _not_ guessing the other candidate-syms of C means that in the houses of C, the already-few cells which have those candidate-syms of C which are not S are now even fewer.

question: could there be a correlation between good guess candidates and cells that could not be removed as givens ("keepers")? Try making a guess-suggester that favours guessing at a cell which sees many keepers.

If a base puzzle is known to be proper, and then a given G is removed to create a derived puzzle, it is known that there is always exactly one solution to the derived puzzle where the cell at G.rmi takes on G.val, and that solution is the solution of the base puzzle. Therefore, if the goal is to check that the derived puzzle is still proper or not, one can remove G as a candidate from all the guess stack frames. Also- I wonder if it would be more efficient to focus the first guess in the stack at the cell of G.

If the guess stack is size N and a guess needs to be popped, currently, the popped guess is ruled out for the guess stack frame beneath it. Is there any way to
-  Is there any such thing as re-ordering the guess stack entries? If so, is there any potential usefulness from doing that?
  - Is it possible that for a given guess stack at a state where it is not yet known whether the guesses result in a valid or invalid solution, that the entries could have been made in a different order (and result in the same knowledge of what candidates still remain)?
    - The FastSolver currently still promises to visit all possible solutions to a puzzle, for which I think the guess stack behaviour as a stack is essential.

### Unavoidable Sets

- For simple size-4 set, one can take two houses of the same line-type and see if firstly they have any pair of syms in vice-versa boxes, and then if they are in the same columns
  1. Num line types: `2`
  2. Num chutes for line-types: `O1`
  3. Num combinations of lines in a chute: `nCr(O1,2)` = `O1!/(2!*(O1-2)!)` = `O1*(O1-1)/2`
  4. Num combinations of two crossing-lines from different boxes: `nCr(O1,2)*O1*O1` = `O3*(O1-1)/2`
  5. Total combinations: `2*O1*O1*(O1-1)/2 * O3*(O1-1)/2` = `O5 * (O1-1)^2 / 2`. Growth with respect to O is `O7`
  6. Possible optimization for step 4: take the two lines of cand-syms, and do a valarray-like sum. (unfortunately, the entries of the valarray need to have tags to point back to the house-cell). then sort/partition entries of the valarray. The crossing-line-pairs will always have the same sum, but entries of the sorted array may not necessarily by crossing-line-pairs. Hm. How about instead of summing, just have a two-entry, sorted array. The work to create the "valarray" is proportional to `O2`. Then to sort the valarray is `O2*log(O2)`. Then to walk it and look for adjacent identical pairs is `O2`. This should have better algorithmic complexity as O grows (roughly slightly over a factor of `O1` better).

What bothers me so much about the above is that it's only for the smallest kind of unavoidable set. I have no idea how design something more efficient than the most naive implementation thinkable for finding other kinds of unavoidable sets. I do wonder what the distribution sampled over many solution grids is of how many of each kind of unavoidable set there is in a solution grid. If the size-4 kind is the most common that would put me at some ease.