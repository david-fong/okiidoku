<!-- SPDX-FileCopyrightText: 2020 David Fong -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
# Interesting Questions for Further Research

- For a given solution grid, is there any correlation between the number of givens in its fewest-givens and most-givens minimal puzzles?
- Over all possible minimal puzzles for a given order size, what is the minimum and maximum number of remaining clues?
  - The minimum number of remaining clues is interesting since it will likely correspond to more difficult puzzles (?)
    - Nope. People on enjoysudoku say that's not true.
    - Well, it's possible that there are benefits for compressed storage? Ie. a research question of: is compression of the average low-clue puzzle smaller smaller than compression of its solution? Ie. when does the space overhead of a bit-array of which cells contain clues get overtaken by the space cost of storing "all the clues"?
      - I'm fairly confident that about half of all clues can usually be removed while retaining puzzle-properness
      -
  - The maximum number of remaining clues is interesting since it will likely be the worst case for compressed storage.

- Challenge to self: find out how to make puzzles with few or many givens.
  - Hypothesis: prioritizing to remove cells that are in a house with more givens / fewer candidate-symbols will create puzzles with few givens and vice versa.
  - It should be true that keeping givens in overlapping UA sets should help reduce the number of givens for a minimal puzzle.

- I'd like to collect some statistics about relations between guess stack depth and num puzcells remaining / total num cand-syms remaining.
  - Perhaps there can be some relation to when to search for larger-sized subsets?

- Can the scramble-invariant property analysis of a grid be used to efficiently estimate the difficulty of a puzzle (or the possible range of difficulties of puzzles generated from a solution)? I believe (and hope) there is potential that the answer is yes. If it is the case, how? And what would the accuracy of the estimation be (error distribution)?
  - If so, could it be used to _create_ probably difficult puzzles?
  - What is the relationship between number of hints and puzzle difficulty?
  - What is the relationship between distribution of hints between houses/chutes and puzzle difficulty?
  - What is the relationship between the relationship between hints and cell relationships and puzzle difficulty?

- Since the stochastic generator cannot deterministically traverse all possible outputs, could it be possible that at a certain point, it becomes easier to find more unique (non-equivalent to previously found) puzzles by continuing a backtracking generator from the output of a stochastic generator's output?

- How does the average number of swaps / swaps-attempts required for the stochastic generator to generate a solution scale with size? what is the distribution of the observed complexity in terms of number of swaps / swap-attempts?