
# Things To Do

- Run some repeated tests and get some performance data.
  - Report findings in the main readme file.
- Try printing size of Tile with and without the fixedVal field
  - If it contributes significantly, find way to take it out?
- Try making traversal order not grid-row-major and see if it improves performance:
  - Hypothesis: Tiles with fewer candidates are like the solution space's dominant bottlenecks. If we were to leave them to the end, we may spend many long advances creating almost-complete solutions that cannot be complete because they violate the bottlenecks (and possibly often in similar ways). If we start with them first, we may be less likely to encounter that problem.
  - Need to give the Tile class an instance field for their og index since that's how we index their group-occupancy vectors and rowBias vectors.
  - Invariant: At the beginning of solution generation, we can filter for all un-seeded tiles and sort them in ascending order of number of remaining candidates. I haven't thought it out 100% yet, but I believe we cannot update this traversal order intermittently (even if we only sort those past the current traversal index) because it could make us miss some outputs or put us in an infinite loop of testing a cycle of outputs. I feel like that's a bit of a bummer, but at this point there are many things I'm not sure of yet, so I'll try to hold that feeling back.

