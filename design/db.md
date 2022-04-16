# Brainstorming Database Ideas

## Goals

### For Full Grids

I want to store as many full grids as I can find. If I later find properties of interest, I want to be able to bucket them by the most useful property. I don't particularly care about having efficient random access (not a requirement), but if I can get it for free, that's great. I want to be able to merge databases efficiently, and if the databases have similar entry ordering requirements (ex. canonical lexicographical, label favouritism), to have the merge result also follow that ordering requirement.

Design approach: Optimize for space, but also keep it reasonably fast to do the serdes. If it seems like a marginal space/time gain from an additional optimizing technique is not worth the additional implementation complexity, don't do it. I realize the wording here is pretty fuzzy. I believe that's okay.

### For Puzzle Grids

I only really care about storing some top percentage of the most difficult puzzles I can find.

Design approach: Since difficult puzzles tend to be sparse, optimize for storing sparse grids. This would make it difficult to get efficient random access, but I don't mind not having it. People in [this sudopedia](http://forum.enjoysudoku.com/giant-sudoku-s-16x16-25x25-36x36-100x100-t6578-150.html) thread working on generating minimal puzzles for large grids were able to remove ~70% of values (similar for various grid sizes). Without the sparse-grid-assumption, 

## Compression for Serialization

## List of Algorithms

Going from least to most compression

## Additional Techniques for Full Grids

For full grids, the boxes along the main diagonal can be removed and are easy to restore.

[Here's a desmos](https://www.desmos.com/calculator/ldtxq1wipc) comparing different levels of compression (including the omission of the main diagonal boxes). The first is where each row gets represented as a number, with each digit being in [0, O2]. The second is where each value takes the minimum bits required to store a number in [0, O2]. The last is where each value takes the minimum number of bytes to store a number in [0, O2]. The ratio between the first and second is fairly small. The first requires bigint support. The second requires parsing sub-byte/non-byte-aligned numbers. I feel like the second could perform faster (?) while still being somewhat simple to implement compared to the first option, and also being fairly more compact than option 3. Just use bitset as an intermediate representation for the serdes.

More improvement can actually be made by looking at intersections between houses and the "worst-case". If serdes traverses entries in row-major order, for some cell C at (row, col), in the "worst case" (where already seen parts of C's houses have maximum similarity), the number of candidates for C is reduced to the minimum of the number of candidates for each house based on the number of already serialized/deserialized cells in that house (O4 - num-cells-already-visited-in-house). For each order, a table can be precomputed of how many candidates remain for that cell. A trade-off can be made to have a compressed table which is slightly sub-optimal compared to the non-compressed table: Have a table with an entry for each boxrow storing a number in the range

### Compression by House

To store an ordered sequence of numbers in `[0 N)`, one can map the sequence to a version where each entry is bumped down by the number of receding values that are less than its value. Then reduce the array to a number in a digit system where each place stores `N - index` values. The maximum value is `N! - 1`.

You can plug the following into desmos to see for yourself (Note that the factorial gets too big for desmos to handle before you can see the naive size/cost jump up at `o = 16`):

```latex
o_{seq}=\left[0...20\right]

B_{house}\left(o\right)=\operatorname{floor}\left(\frac{7+\left(1+\operatorname{floor}\left(\log_{2}\left(o^{2}!-1\right)\right)\right)}{8}\right)\left\{o>1\right\}

B_{naive}\left(o\right)=o^{2}\cdot\operatorname{floor}\left(\frac{7+\left(1+\operatorname{floor}\left(\log_{2}\left(o^{2}\right)\right)\right)}{8}\right)\left\{o>1\right\}

\left(o_{seq},\frac{B_{house}\left(o_{seq}\right)}{B_{naive}\left(o_{seq}\right)}\right)
```

The [x86 BMI2 instruction set's PDEP instruction](https://en.wikipedia.org/wiki/X86_Bit_manipulation_instruction_set#Parallel_bit_deposit_and_extract) can be helpful in speeding up deserialization ([to get the nth set/unset bit](https://stackoverflow.com/questions/7669057/find-nth-set-bit-in-an-int)). [GCC provides a builtin for using it](https://gcc.gnu.org/onlinedocs/gcc/x86-Built-in-Functions.html#x86-Built-in-Functions)
