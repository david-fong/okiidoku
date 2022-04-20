# Brainstorming Database Ideas

## For Full Grids

I want to store as many full grids as I can find. If I later find properties of interest, I want to be able to bucket them by the most useful property. I don't particularly care about having efficient random access (not a requirement), but if I can get it for free, that's great. I want to be able to merge databases efficiently, and if the databases have similar entry ordering requirements (ex. canonical lexicographical, label favouritism), to have the merge result also follow that ordering requirement.

Design approach: Optimize for space, but also keep it reasonably fast and simple to do the serdes (For example, it's probably more optimal to store a minimal puzzle of the solution, but then serialization would require puzzle creation and deserialization would require solving, which violates the speed and simplicity goal). If it seems like a marginal space/time gain from an additional optimizing technique is not worth the additional implementation complexity, don't do it. I realize the wording here is pretty fuzzy. I believe that's okay.

For full grids, the boxes along the main diagonal can be removed and are easy to restore.

### Experiments

[Here's a desmos](https://www.desmos.com/calculator/ldtxq1wipc) comparing different levels of compression (including the omission of the main diagonal boxes). The first is where each row gets represented as a number, with each digit being in [0, O2]. The second is where each value takes the minimum bits required to store a number in [0, O2]. The last is where each value takes the minimum number of bytes to store a number in [0, O2]. The ratio between the first and second is fairly small. The first requires bigint support. The second requires parsing sub-byte/non-byte-aligned numbers. I feel like the second could perform faster (?) while still being somewhat simple to implement compared to the first option, and also being fairly more compact than option 3. Just use bitset as an intermediate representation for the serdes.

More improvement can actually be made by looking at intersections between houses and the "worst-case". If serdes traverses entries in row-major order, for some cell C at (row, col), in the "worst case" (where already seen parts of C's houses have maximum similarity), the number of candidates for C is reduced to the minimum of the number of candidates for each house based on the number of already serialized/deserialized cells in that house (O4 - num-cells-already-visited-in-house). For each order, a table can be precomputed of how many candidates remain for that cell. A trade-off can be made to have a compressed table which is slightly sub-optimal compared to the non-compressed table: Have a table with an entry for each boxrow storing a number in the range

I did some experiments for different traversal paths to compare them in terms of _space_. The script can be found [here](./db_paths_experiment.js). Out of the paths I tried, doing row major with blocks on the non-main diagonal removed was always the second best option by a close margin- the best being fairly more difficult to implement.

## For Puzzle Grids

I only really care about storing some top percentage of the most difficult puzzles I can find.

Design approach: Since difficult puzzles tend to be sparse, optimize for storing sparse grids. This would make it difficult to get efficient random access, but I don't mind not having it. People in [this sudopedia](http://forum.enjoysudoku.com/giant-sudoku-s-16x16-25x25-36x36-100x100-t6578-150.html) thread working on generating minimal puzzles for large grids were able to remove ~70% of values (similar for various grid sizes). Without the sparse-grid-assumption,

The data can start with a bitset of size O4 where bits are set if that cell has a given. The given data can be stored compressed as a bigint.

## Bit Manipulation

The [x86 BMI2 instruction set's PDEP instruction](https://en.wikipedia.org/wiki/X86_Bit_manipulation_instruction_set#Parallel_bit_deposit_and_extract) can be helpful in speeding up deserialization ([to get the nth set/unset bit](https://stackoverflow.com/questions/7669057/find-nth-set-bit-in-an-int)). [GCC provides a builtin for using it](https://gcc.gnu.org/onlinedocs/gcc/x86-Built-in-Functions.html#x86-Built-in-Functions). [Here's another SO post](https://stackoverflow.com/a/59591250/11107541) about using BMI2 with various compilers and targets.
