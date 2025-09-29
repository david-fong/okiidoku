<!-- SPDX-FileCopyrightText: 2020 David Fong -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
# Old Ideas

## Learnings

- I deliberately turned off inter-procedural optimization for debug builds because it was starting to take around 10 seconds each time. The runtime speed impact is slightly below a 10% hit.
  - An alternative approach could be to have a separate build configuration that only builds order=3. It could be for "first-pass" testing, which would probably be faster to do IPO on. I didn't try implementing this because I'm still to lazy to figure out / learn how to make a good CMake interface for selecting grid orders to compile.

## Not Possible (?)

- I noticed that the exported symbols of the dynamic library weren't getting grouped by order; I though this could be a bad thing for code locality.
  - Tried out [this](https://stackoverflow.com/questions/6886567/how-does-gcc-decide-what-order-to-output-assembly-functions-in). Made a BIST function that calls all the things and see if it will cause monomorphic functions to get grouped together. See if exporting the BIST is required for the ordering to be done.
  - It didn't work. Tried making the dynamic library static and it still didn't work. I think I should stop worrying about it (don't micro-optimize things that won't have noticeable effect).
- what: try using the GCC "section" function attribute to put same-order code in the same section. Ex. "text-okiidoku-o3". Test this on an older commit and see how it goes. May want to get rid of usages of `[[gnu::hot/cold]]`.
  - motivation: code locality.
  - why it doesn't work: can't use template arguments in attribute string-argument values. :(
  - An alternative might be to do the template instantiations all together, since the positioning of machine code functions seems to be based around the positioning of the function definitions. I don't like this options as much as what I have right now: it would mean any change to an implementation would recompile everything (since all instantiations would need to be in a same TU).
  - another alternative would be to use PGO. That's probably what I'll go with once I figure out how to do it.

## Things Tried that I Reverted

- separating the monomorphic containers and algorithms each into their own dynamic library and making another separate library for the visitor pattern.
  - It wasn't a bad idea. I just didn't like how it resulted in a bunch more files and made things harder for _myself_ to use within the project. Nobody is using this project right now; it doesn't make sense to me to inconvenience myself for the sake of nobody.
  - If somebody in the future uses this project and wants this kind of change for a good reason and for _some_ reason I decide I want to support them, at least I know how to do it now.

## Never Got Around To These

- (switched to using stochastic search generator before trying these) some diagnostics to try rendering:
  - A scatter chart showing max-dead-ends vs. num operations. (only caring about success results)
  - a bar graph where each bar counts the number of ResultViews that had a progress (or a furthest coord with a non-zero backtrack count) within the range for that bar's "bin". (to see "how far" aborted generations usually get).
  - comparing the average heatmap of aborted vs successful generations.
    - See if there are clear differences/patterns in where they usually spike in backtracking. Perhaps this can be used to inform more sophisticated thresholds for each genpath.

## Questionable API Design Goodness

- questionable idea: giving the callback in batch a dedicated mutex, or no mutex at all and leaving it up to the caller. need to consider how likely it is that the bulk of a callback will need synchronization.
  - somewhat against this idea because
    - the new stochastic generator is so fast that I have a hard time seeing anyone wanting more optimization on top of it.
    - not providing automatic mutex for the callback seems like a bit of an api footgun? I think in the average use-case it would probably be more of an annoyance than something desirable.
  - another idea I had along the same lines: make a buffering adapter for batch callbacks. use destructor to flush.
    - If a user wants this, they can still write it themselves.

- ? Refactor names to use terminology suitable for more than just 2 dimensions? Ex. in 2D: row -> `d0i` (as in "dimension-zero index"), col -> `d1i`. But doing so would imply that I'm going to support multiple dimensions... and that's a huge can of worms.
  - I don't really see myself attempting to do a 3d version of sudoku, and doing this seems like it would make the 2d code harder to read.

- [it is allowed in c++ to have template functions and non-template functions with the same name. here are the resulting rules](https://stackoverflow.com/a/16865452/11107541). Could this be used to put the algorithm functions under the same namespace (not in separate "mono" and "visitor" namespace)?
  - cppcoreguidelines recommends placing functions operating on a class in the same namespace as the class, which allows ADL. Whether or not ADL is a good thing is a separate discussion, but I'm still on board with this recommendation.

## No Longer Needed

- "smarter"/greedier backtracking: backtracking may be occurring frequently at a coord because of values much earlier in the genpath progress. (wikipedia "backskipping")
  - (no longer needed because stochastic search generator is much faster than backtracking. Would be better to keep backtracking's basic implementation's quality of never skipping possible outcomes)
  - https://wikipedia.org/wiki/Backjumping
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

### Canonicalizer

- See [the bottom of the canonicalization readme](./morph.md)
