# Puzzles

## Puzzle Generation

Goals: Generate minimal, difficult puzzles. TODO: by what definition of difficulty?

## Solver

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