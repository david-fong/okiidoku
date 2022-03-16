# Old Ideas

## Never Got Around To These

- (switched to using stochastic search generator before trying these) some diagnostics to try rendering:
  - A scatter chart showing max-dead-ends vs. num operations. (only caring about success results)
  - a bar graph where each bar counts the number of ResultViews that had a progress (or a furthest coord with a non-zero backtrack count) within the range for that bar's "bin". (to see "how far" aborted generations usually get).
  - comparing the average heatmap of aborted vs successful generations.
    - See if there are clear differences/patterns in where they usually spike in backtracking. Perhaps this can be used to inform more sophisticated thresholds for each genpath.

## No Longer Needed

- "smarter"/greedier backtracking: backtracking may be occurring frequently at a coord because of values much earlier in the genpath progress.
  - (no longer needed because stochastic search generator is much faster than backtracking. Would be better to keep backtracking's basic implementation's quality of never skipping possible outcomes)
  - https://en.wikipedia.org/wiki/Backjumping
  - backtracking is less likely to occur when other coords in the same house as the stuck coord that have different house types have the same value (overlapping has_mask). Can make an array like a count version of has_mask counting the times a value is taken in each house seen by the stuck coord.
    - Indicators of "bad packing" (lack of overlap): how many more bits are in the coord's has_mask than the has_mask of the house with the most bits in its has_mask?
    - Each coord has a level of prone-ness to bad packing: To get it, walk the gen path, and at each cell, accumulate that cell's coord to a pool, then walk the pool and count how many coords are seen by the current coord. Take `max(count_seen - O2, 0)`.
  - stupid solution: backtrack a distance proportional to the backtrack count of a cell: distance equals `1+floor(log_{base}(backtrack_count))`, where a logical value for `base` is `O2`, since each cell can try up to that many values (distance of backtrack is proportional to the max effort to get back to the backtrack origin), but that is very slow to take effect. If assuming the average probability of going forward at a cell is 0.5, then use `base = O2/2`? stupid because progress does not correlate to coords that are in the same house as the backtrack origin. Can make it so that it only decrements the remaining backtrack distance counter when backtracking from a coord in the same house as the backtrack origin, and then treating an abort as when hitting `progress == 0` with a non-zero remaining backtrack distance counter?
    - The assumption of 0.5 chance of going forward at any coord is not very realistic. A better approximation might take into account a cell's prone-ness to bad packing with the given genpath.

## Ideas That Seem To Not Have Worked / Are Impractical

These didn't end up doing the thing I wanted / thought might happen.

- refactor template expansions and tweak config header to co-locate same-order expansions while also giving a somewhat convenient way to control what library features to include? Or look into a way to get linker to automatically do this.
  - started the refactor and then decided to see if there was perf improvement when compiling for only one order. No improvement was observed. Refactoring effort abandoned for added complexity. If there is other merit found in the future, the work is stashed in the `collate-so-symbols` branch.

### Generator

- Using https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size for batch's ThreadFunc generator field. Didn't seem to have any effect. Probably because the generator struct is larger than 64 bytes :P.

- Try making traversal order not grid-row-major and see if it improves performance:
  - Hypothesis: Cells with fewer candidates are like the solution space's dominant bottlenecks. If we were to leave them to the end, we may spend many long advances creating almost-complete solutions that cannot be complete because they violate the bottlenecks (and possibly often in similar ways). If we start with them first, we may be less likely to encounter that problem.
  - Update: I think this didn't work because I was doing naive backtracking. If I improve the backtracking logic. Now I need to remember/find out which traversal paths should make better usage of this and then test it out.
    - hm. but even with skipping backtracks when not-in-same-house as the backtrack origin, there's the projected cost of reaching backtrack origin again. maybe that's why deal_row_major is so bad?

### config_auto_canonicalizer

- See [the bottom of the canonicalization readme](./morph.md)
