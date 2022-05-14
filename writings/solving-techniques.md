# Solving Techniques

[sudopedia](https://www.sudopedia.org/wiki/Solving_Technique)

Rules locally derived from the one rule.

## Locked Candidates

- [sudopedia](https://www.sudopedia.org/wiki/Locked_Candidates)

Given intersection `I` between some line `L` and box `B`, if either house `L` or `B` can only put symbol `S` in `I` (ie. none of its cells outside `I` have `S` as a candidate), then the other house can also only put `S` in `I` (ie. it can remove `S` from the candidate sets of all its cells which are outside `I`).

## Naked Subsets

- [sudopedia](https://www.sudopedia.org/wiki/Naked_Subset)
- A technique that finds cells that "desperately" require certain symbols, and deduces that other cells cannot have those symbols.
- If in a house `H` there are `N` cells `CELLS` where the union of their candidate-symbol sets `CANDS` is exactly size `N`, all cells in `H` and not in `CELLS` can remove everything in `CANDS` from their candidate-symbol sets.
- Recommended: If one wishes to minimize the number of rounds of applying this technique, try to find instances with small `N`: if within a `CELLS`, there is a smaller such set that also allows for this technique, use the smaller set.
- Note: for a proper puzzle, it is impossible to have a `CANDS` with size less than `N`.

## Hidden Subsets

- [sudopedia](https://www.sudopedia.org/wiki/Hidden_Subset)
- A technique that finds symbols that "desperately" require certain cells, and deduces that other symbols cannot have those cells.
- If in a house `H` there are `N` symbols `SYMS` where the union of their candidate-cell sets `CANDS` is exactly size `N`, all cells in `CANDS` can remove from their candidate-symbol sets everything that is not in `SYMS`.
- Recommended: If one wishes to minimize the number of rounds of applying this technique, try to find instances with small `N`: if within a `CELLS`, there is a smaller such set that also allows for this technique, use the smaller set.
- Note: for a proper puzzle, it is impossible to have a `CANDS` with size less than `N`.

## Fish

- [sudopedia](https://www.sudopedia.org/wiki/Fish)


---

# Making Hard Puzzles

How is it done?

- Perhaps by searching for the next given to remove that retains the most symbol and cell candidates?

- Perhaps there is some correlation to the properties currently being used for canonicalizing labels?