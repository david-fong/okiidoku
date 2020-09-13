
# Things To Do

[Try this?](https://codegolf.stackexchange.com/questions/190727/the-fastest-sudoku-solver)

Ex. `000000010400000000020000000000050407008000300001090000300400200050100000000806000`

## Higher Priority

- Make giveup thresholds settable via CLI, and allow setting to "default".
- give a red message when trying to continue and nothing is left to be found.
- Update readme to talk about the four problems in Sudoku:
  - solution (verification, reconstruction, compression, generation)
  - (reconstruction is better known as puzzle solving, and compression as puzzle generation).

## Lower Priority

- Look at ways to use `__builtin_prefetch`.
- Make non-prompt output text print to cout and to outStream (if outStream is not same as cout).
  - This includes the preamble of parameters that were parsed.
  - Investigate if it is simplest to do this with a macro :P.
  - Or just do it explicitly in important places like `Solver::print`, and make them take `isPretty` as an argument.

## "Failed" Experiments

These didn't end up doing the thing I wanted / thought might happen.

- Try making traversal order not grid-row-major and see if it improves performance:
  - Hypothesis: Tiles with fewer candidates are like the solution space's dominant bottlenecks. If we were to leave them to the end, we may spend many long advances creating almost-complete solutions that cannot be complete because they violate the bottlenecks (and possibly often in similar ways). If we start with them first, we may be less likely to encounter that problem.
  - Need to give the Tile class an instance field for their og index since that's how we index their group-occupancy vectors and rowBias vectors.
  - Invariant: At the beginning of solution generation, we can filter for all un-seeded tiles and sort them in ascending order of number of remaining candidates. I haven't thought it out 100% yet, but I believe we cannot update this traversal order intermittently (even if we only sort those past the current traversal index) because it could make us miss some outputs or put us in an infinite loop of testing a cycle of outputs. I feel like that's a bit of a bummer, but at this point there are many things I'm not sure of yet, so I'll try to hold that feeling back.

- Use `__builtin_popcount(occmask)` or `__builtin_ctz(!occmask)` to have special case for when the occmask is full:
  - In that case, just backtrack. Don't go through the search loop because we won't find anything in it.
  - What about if length - popcount is 1?
    - Then I could just set the value right away. This would need a c++ builtin for arbiting a bit.

## Bookmarks

```text
https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
```
