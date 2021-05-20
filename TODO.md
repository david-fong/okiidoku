
# Things To Do

## Higher Priority

## Lower Priority

## "Failed" Experiments

These didn't end up doing the thing I wanted / thought might happen.

- Try making traversal order not grid-row-major and see if it improves performance:
  - Hypothesis: Tiles with fewer candidates are like the solution space's dominant bottlenecks. If we were to leave them to the end, we may spend many long advances creating almost-complete solutions that cannot be complete because they violate the bottlenecks (and possibly often in similar ways). If we start with them first, we may be less likely to encounter that problem.
  - Update: I think this didn't work because I was doing naive backtracking. If I improve the backtracking logic. Now I need to remember/find out which traversal paths should make better usage of this and then test it out.

- Use `__builtin_popcount(occmask)` or `__builtin_ctz(!occmask)` to have special case for when the occmask is full:
  - In that case, just backtrack. Don't go through the search loop because we won't find anything in it.
  - What about if length - popcount is 1?
    - Then I could just set the value right away. This would need a c++ builtin for arbiting a bit.

## Bookmarks

```text
https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
```
