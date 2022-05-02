# House Masks

## Small and Fast Variants

Should there be "small" and "fast" variants? Ie. When `O < 8`, should the smallest int type with `O2` bits be used?

- I don't see myself having house masks pertaining to multiple grids in memory at the same time; I don't even have any current api that exposes utilities for reading house masks. They're only currently for the internal things that need them, which only need them transiently. And I know for row-major-traversal usages, I can optimize by only storing one house for rows and `O1` houses for boxes in memory at a time. So I don't think it's a big deal if I use a u64 granularity of storage. I could perhaps consider optimizing at least for when u32 is sufficient.