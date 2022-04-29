# Interesting Questions for Further Research

- Can the scramble-invariant property analysis of a grid be used to efficiently estimate the difficulty of a puzzle? I believe (and hope) there is potential that the answer is yes. If it is the case, how? And what would the accuracy of the estimation be (error distribution)?
  - If so, could it be used to _create_ probably difficult puzzles?
  - What is the relationship between number of hints and puzzle difficulty?
  - What is the relationship between distribution of hints between houses/chutes and puzzle difficulty?
  - What is the relationship between the relationship between hints and cell relationships and puzzle difficulty?

- Over all possible minimal puzzles for a given order size, what is the minimum and maximum number of remaining clues?
  - The minimum number of remaining clues is interesting since it will likely correspond to more difficult puzzles (?)
  - The maximum number of remaining clues is interesting since it will likely be the worst case for compressed storage.

- Since the stochastic generator cannot deterministically traverse all possible outputs, could it be possible that at a certain point, it becomes easier to find more unique (non-equivalent to previously found) puzzles by continuing a backtracking generator from the output of a stochastic generator's output?

- How does the average number of swaps / swaps-attempts required for the stochastic generator to generate a solution scale with size? what is the distribution of the observed complexity in terms of number of swaps / swap-attempts?