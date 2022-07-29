<!-- SPDX-FileCopyrightText: 2020 David Fong -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
# Generator Notes

## Backtracking

- can deterministically cover all solutions of a puzzle without revisiting already-visited solutions.
  - can be used to find out how many solutions a puzzle has.

- when not combined with deductive methods, is very slow for larger grids

## Stochastic Search

I learned about this from [this forum thread](http://forum.enjoysudoku.com/giant-sudoku-s-16x16-25x25-36x36-100x100-t6578-120.html#p259504) where I later [made a post describing my optimizations](http://forum.enjoysudoku.com/giant-sudoku-s-16x16-25x25-36x36-100x100-t6578-150.html#p318577).

### The Basic Idea

Start with valid rows. Continually pick a random row and two random cells in the row; if swapping them would not cause their columns and boxes to be missing more symbols, swap their symbols. Do this until the grid is valid. A greedy algorithm that performs advances that are pareto-equivalent or a pareto improvement.

### My Optimizations

First make all boxes valid while allowing invalid columns. Once all boxes are valid, make all the columns valid. This constrains the swaps to be more "intentional" and aligned with the rule of sudoku.

Improve cache locality by doing one chute at a time: when working on boxes, completely satisfy the boxes of the first horizontal chute before moving to the next one; when working on columns, completely satisfy all columns of the first vertical chute before moving to the next one. This results in less data being worked on at any time.

When storing counts of how many copies of a symbol a house has, for the 2D array where one dimension is for the `O2` different symbols, and the other is for the `O1` houses currently being made valid, make the outer dimension be the one for symbols. Rationale: cache locality. As grid-order increases, `O2` grows much faster than `O1`. Every swap operation involves four entries in the 2D array, being four "corners" in a sub-rectangle of the array. Putting the smaller dimension as the inner one increases the likelihood of getting pairs of those four "corners" in the same cache line (the best case becomes two cache misses instead of four).

Note: I explored the difference in starting with valid boxes instead of valid lines (rows/columns). Using a counter, I found that it took fewer operations to go from having one polarity of lines valid to also having boxes valid than from having only boxes valid and then getting one polarity of lines to also be valid. Ie. It should be less optimal to start only with valid boxes (and then get both cols and rows valid).

### Observations and Properties

- scales well for large grids for random generator purposes.

- I have no clue as to whether it is guaranteed to terminate. could it get stuck in a local optima? If so, how often does that happen (probability)?

- naively extending the current implementation to solve puzzles will not terminate if the puzzle is not solvable.

- if used for solving, must be the final step in a "hybrid solver" pipeline (even if used as an intermediate step as a brute force part, it wouldn't be deterministic, and can hardly be controlled), which may not be the most optimal approach compared to combining deductive reasoning with backtracking. It is also unable to measure "difficulty" of a puzzle according to deductive reasoning techniques.

- hypothesis (not verified): a stochastic approach may not shine (in terms of throughput (which is pretty much all that is cared about)) when used as part of a puzzle solver; when used as a generator, it only needs to randomly land itself on one of the myriad of possible "solutions" of the empty grid "puzzle". With a proper puzzle, there is only one success state to land on.