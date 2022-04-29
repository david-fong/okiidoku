# Generator Notes

## Misc Existing Literature

- [Math Cornell - There is no 16 clue sudoku](https://arxiv.org/abs/1201.0749)

- http://forum.enjoysudoku.com/giant-sudoku-s-16x16-25x25-36x36-100x100-t6578-120.html#p259504
  - these people have generators that apparently scale very well. Try implementing their algorithms?
  - [donald knuth's dancing links paper](https://www.ocf.berkeley.edu/~jchu/publicportal/sudoku/0011047.pdf)
    - a blog-like post with another illustration https://garethrees.org/2007/06/10/zendoku-generation/#section-4
  - a paper http://www.dudziak.com/ArbitrarySizeSudokuCreation.pdf
  - patterns game strats: http://forum.enjoysudoku.com/patterns-game-strategies-t6327.html

## Some Generator Notes

- backtracking generator
  - can deterministically cover all completions of a grid without revisiting already-visited outcomes.
    - can be used to find out how many solutions a puzzle has.
  - very slow for larger order grids

- stochastic search
  - scales well for large grids for generator purposes.
  - I have no clue as to whether it is guaranteed to terminate. could it get stuck in a local optima? If so, how often does that happen (probability)?
  - naively extending the current implementation to solve puzzles will not terminate if the puzzle is not solvable.
  - if used for solving, must be the final step in a "hybrid solver" pipeline (even if used as an intermediate step as a brute force part, it wouldn't be deterministic, and can hardly be controlled), which may not be the most optimal approach compared to combining deductive reasoning with backtracking. It is also unable to measure "difficulty" of a puzzle according to deductive reasoning techniques.
  - see [here](http://forum.enjoysudoku.com/giant-sudoku-s-16x16-25x25-36x36-100x100-t6578-150.html#p318577) for my post on a high-level algorithm description