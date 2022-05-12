
- Grids must not needlessly waste space in memory.
  - Use the smallest integer size for each entry.
  - Further space optimization is possible at the expense of implementation complexity and runtime cost:
    - Use bitset-like abstraction to use minimal bit0width for each entry
    - Use bignum with a `O2`-ary number system.

- Try to avoid dynamic memory allocations and deallocations for possibly-hot code.

- I don't currently see much need for enabling spatial locality between multiple grids.

- Unions would enable grids to be allocated on the stack, but I also don't like the idea that using unions to represent variable-size grids would waste space for small grids when allocating multiple of them and compiling with support for a very large grid size...