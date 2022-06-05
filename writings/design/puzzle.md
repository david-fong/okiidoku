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

### Making Guesses while Before Applying Found Deductions

My solver engine has a contract that guesses must not be made while the candidate elimination queue is not empty.

This makes rational sense: With the chosen data representation (or any chosen data representation for that matter), there is computational cost to deduce whether a candidate can be eliminated by almost any deductive technique. A guess is a gamble: it can be right or wrong- whereas a deduction can only "be wrong" if it was made on the basis of preceding guesses (ie. the "wrongness" of a deduction is really the discovery that some previous guess was incorrect). On top of that- any deduction (whether based on prior guesses or not) strictly decreases the probability of future guesses being wrong (a deduction decreases the number of candidates all over the representation; the number of candidates bounds the number of possible guesses; the number of possible guesses bounds the number of incorrect guesses). So why would you make a gamble if you have available options with less inherent risk?- especially considering that you needed to spend effort to discover those options. Why would you ever make a guess that would be directly ruled out by a candidate deduction in the queue? The answer is- you wouldn't (shouldn't); there is never a good reason to make a guess while deduced candidate eliminations are queued.

Okay- but why a contract? Why not just a guideline (a strong recommendation)?

There is some necessary prerequisite discussion before I answer that. Here's the question to kick it off: What should happen to queued candidate eliminations when a guess is unwound from the guess stack? The prerequisite question to that question is: How do you know which guesses on the guess stack a newly found (deduced) candidate elimination depends on? (a non-dependence on a guess means that the deduction could be made without assuming on the validity of that guess) My answer is that I don't know. In the case where a user of the engine always consumes the contents of the deduction queue before making a guess, then it is safe to say that any deduction made by that engine user is made on the basis of (ie. depends on) _all_ previous guesses that were on the guess stack when that deduction was made/found. I _do_ know that that case forms the worst case of what could be true if guesses are made while the deduction queue is not empty. If no effort is made (and such an effort would need to be made at runtime) to determine a narrower bound on a deduction's set of possible dependencies on preceding guesses, then to preserve correct state of the engine, the unwinding of any guess must also unwind all deductions in the queue that are not known to be independent of that guess (ie. all deductions that possibly depend on that guess) (ie. all deductions that were made _after_ making that guess).

Remember: If the deduction queue is always emptied before making a guess, then at all times, every queued deduction depends on every guess in the guess stack, and unwinding the stack (popping the top entry) _necessitates_ invalidating (removing) all queued deductions. What's so great about that? It's trivial to implement, achieves correctness, and never does anything wasteful (based on that condition of empty-queue-before-any-guess).

But what if it's a guideline and not a contract? How do you maintain correctness?

1. Automatically consume all queued deductions inside the function that is called to push a guess. This is simple and correct. What's not to like about it? This: application of deduction can enable further deduction that was not previously possible. To immediately make a guess when further deduction may be possible- and to do so in a way that is outside of the engine user's control- is illogical. This solution could be described as the engine "guarding itself from illogical usage by spitefully reciprocating with more illogical behaviour". None of the intended use-cases for the engine would find such behaviour useful, and documenting such behaviour could even mislead someone trying to use the engine for their own use-case to believe that such behaviour is intended to be helpful to them. There is nothing about this solution that violates correctness. It's just yucky and I don't like it.

1. Add a field to deduction queue entries that contains the size of the guess stack at the time it was enqueued (a "foundation-guess" value). When unwinding a guess entry, only those queued deductions that have a foundation-guess equal to the guess being unwound should be removed. Still pretty simple, right? And it even sounds _cool_ because it handles this weird case that people might not have thought of! Here's what's not to like about it: we established that that weird case is wasteful. The engine is designed as a lower-level primitive to build two actual-user-facing-solvers with very different goals but which have no intention to use the engine in any way that is obviously wasteful. Why should the engine pessimize its memory-consumption and speed as seen by its intended use-cases for a use-case that is inexcusably illogical and wasteful? Nay. Death be to this proposed solution and dishonour on its cow.
  - Another complication I just realized: if you push a guess and then apply something that was found before that guess and then no solutions remain, then either you leave the engine user to re-find them, or you have to implement some way to push them back when unwinding.

1. Do the same thing as if it was a contract: when unwinding a guess, clear the deduction queue. It would remove queued deductions that the wasteful user didn't use before making a guess. Fine. Let it be on their heads. Play stupid games, win stupid prizes.

In the end, I actually decided to make this a contract instead of a guideline. Rationale: There's never a good reason for an engine user to ever break the guideline. It really does end up simplifying the engine implementation: the engine has queues for found candidate eliminations. The CellClaimSym queue is special due to the cell-major representation. I chose to make the finding of its entries "passive" (done automatically during the process of applying any kind of found candidate elimination). If I want to keep it completely passive with _no_ finder function _and_ allow guesses while its queue is not empty, then I can't automatically clear it upon unwinding since it will never regenerate the lost entries. I've already explained why I don't like the first two solutions (which is why I don't want to implement them).

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

question: could there be a correlation between good guess candidates and cells that could not be removed as givens ("keepers")? Try making a guess-suggester that favours guessing at a cell which sees many keepers.

If a base puzzle is known to be proper, and then a given G is removed to create a derived puzzle, it is known that there is always exactly one solution to the derived puzzle where the cell at G.rmi takes on G.val, and that solution is the solution of the base puzzle. Therefore, if the goal is to check that the derived puzzle is still proper or not, one can remove G as a candidate from all the guess stack frames. Also- I wonder if it would be more efficient to focus the first guess in the stack at the cell of G.