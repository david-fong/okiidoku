<!-- SPDX-FileCopyrightText: 2020 David Fong -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
# Rethinking the Solver Engine

## Important Questions

Which information sets must communicate, and which can be

## Useful information formats for analysis

### Naked Subsets

- For a given house, for each cell, what symbols can it have?
- It is useful to have a list of tags pointing to cells sorted by the number of candidates each cell has: When searching for subsets of a given size, ignore cells which by themselves already have too many candidates. This tag list does not necessarily need to be updated "live". It can be updated just before performing this kind of analysis.

- For each house:
  - up to O2 O2BitArr to store candidate lists
  - O2 o2x_t either containing the single candidate, or a pointer to it in the O2BitArr pool.
  - cache:
    - O2 structs of an o2x_t for the count-1 (assuming that analysis is never called when a count is zero, since that implies un-solvability), and an o2x_t pointing to .
  - the O2BitArr pool can be sorted according to logical order, or by cand count. Just make sure that when doing gc, if sorting by cand count, use an up-to-date version of the cache info to order the moves so that no information is accidentally destroyed in the process.

- for each house, map from symbol to which cells it can go in.
