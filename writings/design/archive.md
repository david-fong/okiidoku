<!-- SPDX-FileCopyrightText: 2020 David Fong -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
# Brainstorming Archiving Ideas

## For Full Grids

### Goals

- Persistently store every single grid generated.
- No duplicate entries (conversion of grids to some canonical form will be helpful for this)
- Store grids in a highly compressed form that can have an efficient serdes implementation.
- Allow quickly checking if a grid that is _not_ stored in the ARCHIVE is equivalent to one that _is_ stored in the ARCHIVE.
  - TODO actually I'm not sure if I still want this. It would be useful if I've generated enough grids to the point that on average I begin generating ones that I've already found, but I'm not sure how likely/when I'll get to that point. Otherwise, when I merge, I can just skip entries that are duplicate with the last-pushed entry to the merged ARCHIVE.
- I want to be able to merge archives efficiently, and if the archives have similar entry ordering requirements (ex. canonical lexicographical, label favouritism), to have the merge result also follow that ordering requirement.

### Non-Goals

These properties are not direct requirements, but they may end up being needed in order to satisfy / better satisfy another requirement.

- Efficient random access
- Sorted entries (useful if binary search is desirable)
- Efficient entry insertion

My personal end goal of storing all these grids is to have a large set of grids to use for when I later start working on puzzle generation and trying to make very hard puzzles. Perhaps other people can find other uses, but I can't think of any right now.

### Design Approach

Optimize for space, but also keep it reasonably fast and simple to do the serdes (For example, it's probably more optimal to store a minimal puzzle of the solution, but then serialization would require puzzle creation and deserialization would require solving, which violates the speed and simplicity goal). If it seems like a marginal space/time gain from an additional optimizing technique is not worth the additional implementation complexity, don't do it. I realize the wording here is pretty fuzzy. I believe that's okay.

For full grids, the boxes along the main diagonal can be removed and are easy to restore.

In order to be able to quickly check if a generated grid is already in the ARCHIVE by binary search, a comparison function needs to either be defined for grids in serialized or deserialized form. If defined on the deserialized form, the binary search would need to operate on deserialized entries, which would be inefficient unless the whole ARCHIVE needs to be deserialized into main memory for some other requirement (and currently there is no such requirement). Defining the comparison function serialized/compressed grids feels weird, but here there's actually a good contextual reason for it.

### Fixed-Size by Worst-Case Compression Experiments

[Here's a desmos](https://www.desmos.com/calculator/ldtxq1wipc) comparing different levels of compression (including the omission of the main diagonal boxes). The first is where each row gets represented as a number, with each digit being in [0, O2]. The second is where each value takes the minimum bits required to store a number in [0, O2]. The last is where each value takes the minimum number of bytes to store a number in [0, O2]. The ratio between the first and second is fairly small. The first requires bigint support. The second requires parsing sub-byte/non-byte-aligned numbers. I feel like the second could perform faster (?) while still being somewhat simple to implement compared to the first option, and also being fairly more compact than option 3. Just use bitset as an intermediate representation for the serdes.

More improvement can actually be made by looking at intersections between houses and the "worst-case". If serdes traverses entries in row-major order, for some cell C at (row, col), in the "worst case" (where already seen parts of C's houses have maximum similarity), the number of candidates for C is reduced to the minimum of the number of candidates for each house based on the number of already serialized/deserialized cells in that house (O4 - num-cells-already-visited-in-house). For each order, a table can be precomputed of how many candidates remain for that cell, or it can be generated on the fly each time.

I did some experiments for different traversal paths to compare them in terms of _space_. The script can be found [here](./solution_serdes_paths_experiment.js). Out of the paths I tried, doing row major with blocks on the non-main diagonal removed was always the second best option by a close margin- the best being fairly more difficult to implement.

### Non-Fixed-Size Entries for More Compression

All the experiments above use worst-case values for candidate counts. This automatically gives such entries all the same number of bits, which also gives random access for free. _But_...

Further compression can be gained by using the actual number of possible candidates remaining for a cell based on the actual values of same-house cells already processed during serialization. I'm not sure how much could be gained from this on average.

Doing so would mean entries don't all have the same number of bits as each other, and so the ARCHIVE would have to include link-to-next-entry information for each entry. The links could be absolute or relative. For absolute, the bit cost of this is proportional to the number of unique grids for a grid size and the average number of bits required to store a serialized grid, but the computational effort to generate higher order grids could also be taken into consideration to make an argument for link info with bit-cost invariant with respect to grid size. For relative links, the bit cost is at most the bit cost of storing the the value of the number of worst-case bytes taken by an entry.

If absolute links are used, storing the link map like a "table of contents" separate from and before the actual grid contents of the ARCHIVE would be helpful for doing the binary search. If each link were instead stored next to their corresponding entry, binary search would first require scanning the ARCHIVE to extract the links to contiguous main memory. If relative links are used, the table of contents still needs to be translated to an absolute form, so it isn't critical whether the links are stored as a separate ToC, or interleaved with each entry, though the ToC means less file scanning when constructing the absolute links into main memory for processing.

For this approach to be worth to be worthwhile in terms of _persisted storage space used_, the average number of bytes saved compared to the worst case must be equal to or greater than the number of bytes to store an entry's link (relative links for orders below 6 only consume one byte; 2 bytes for orders up to 17, etc.). Hopefully that's true.
TODO.try test out the above.


## For Puzzle Grids

I only really care about storing some top percentage of the most difficult puzzles I can find.

Design approach: Since difficult puzzles tend to be sparse, optimize for storing sparse grids. This would make it difficult to get efficient random access, but I don't mind not having it. People in [this sudopedia](http://forum.enjoysudoku.com/giant-sudoku-s-16x16-25x25-36x36-100x100-t6578-150.html) thread working on generating minimal puzzles for large grids were able to remove ~70% of values (similar for various grid sizes). Without the sparse-grid-assumption,

The data can start with a bitset of size O4 where bits are set if that cell has a given. The given data can be stored compressed as a bigint.

## Bit Manipulation

- [how to get nth set bit using PDEP](https://stackoverflow.com/questions/7669057/find-nth-set-bit-in-an-int)
- [wikipedia for Intel x86's BMI2 instruction set](https://wikipedia.org/wiki/X86_Bit_manipulation_instruction_set#Parallel_bit_deposit_and_extract)
- [An SO post](https://stackoverflow.com/a/59591250/11107541) about using BMI2 with various compilers and targets
- how to use:
  - [GCC builtin for PDEP](https://gcc.gnu.org/onlinedocs/gcc/x86-Built-in-Functions.html#x86-Built-in-Functions)
  - [MSVC intrinsics for x86-64](https://docs.microsoft.com/en-us/cpp/intrinsics/x64-amd64-intrinsics-list?view=msvc-170)
  - [Emscripten Intrinsics](https://emscripten.org/docs/porting/simd.html). I don't see BMI2 stuff here... maybe the bit scan (shift by count trailing zeroes) fallback can be used.
- There isn't an easy, zero-dep way to detect target arch that works for all compilers. [Boost has a header for this though](https://www.boost.org/doc/libs/1_79_0/libs/predef/doc/index.html#_boost_arch_x86_64). If we don't want to add those whole of boost as a dependency, it lists out how to copy just the thing we want.
