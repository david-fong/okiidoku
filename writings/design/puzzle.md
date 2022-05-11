# Puzzles

## Puzzle Generation

Goals: Generate minimal, difficult puzzles. TODO: by what definition of difficulty?

## Solver

There are two paths here: one is to create a solver that purely optimizes for speed/throughput ("fast solver"), and the other is to prioritize using human solving strategies and be able to rank the difficulty of the puzzle based on the difficulty of techniques used ("reasoning solver").

Another design decision: should the solver be able to detect when there are multiple solutions and no solutions? Or should it assume proper puzzles?

I want to create a solver that can detect when there are no solutions and walk all possible solutions.

My first priority is to get a solver that purely optimizes for speed/throughput. I may try to create a "sudoku explainer" (puzzle difficulty rater) in the future.