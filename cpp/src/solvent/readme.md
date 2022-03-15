
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
  - scales well for large grids
  - I have no clue as to whether it is guaranteed to terminate. could it get stuck in a local optima? If so, how often does that happen (probability)?
  - naively extending the current implementation to solve puzzles will not terminate if the puzzle is not solvable.