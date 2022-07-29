<!-- SPDX-FileCopyrightText: 2020 David Fong -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
# Finding Subsets

Date: 2022/June/13

Did some comparison for finding subsets. Used the puzzle maker with order 5, and waited until 249 puzcell candidates remained. For no subset finding, it took 2m48s. For full subset finding with empty guess stack, it took 2m28s. For finding subsets of size 2 regardless of guess stack depth, it took 1m48s.

For effort parameterization, what's a good way to do it?

- Take a max-subset-size param to dictate up to what subset size of either type to try to find.
  - This is nice and simple to implement, but there's an alternative that has closer correspondence to constraining the "combinatorial explosion": If the subset that we are trying to partition by finding a sub-subset in it is very close in size to the naked subset size being searched, then the number of combinations is also relatively small.
- Take a param to dictate how many combinations to try.
  - Also simple to implement (on the callee side) and can correspond closer to constraining combinatorial explosion.
- Take a param P to dictate effort, where for N less than O2, it will search for subsets of up to the size K where the sum of the number of combinations it would have to try over all those subset sizes in the worst case would be less than or equal to the number of combinations if N was O2 and K was P.

## Implementation Research Questions

- what if we made subset-search also find and apply singles?
  - If specializations for finding singles co-exist with the subset finding, and subsets also find and apply singles, then there should be a documentation note that using the subset finders means the singles finders (presumably?) make no sense to use at the same time.

- what optimizations could be made to quickly check if entire houses have experienced no change? how often would such optimizations come into effect? would there be a net benefit? if so, how much?
- when a subset has been found, we currently always go back to subset-size index 0, but is there anything smarter we could do?
  - If you find a naked subset, the resulting eliminations in the new partition can illuminate new naked subsets, but I don't think it can illuminate new hidden subsets.
  -

- What ways could be explored to speed up the OR-ing together of cand-sym masks when searching for subsets?
  - A tree-like structure where middle layers cache some pre-computed ORs of partial combinations?

- when searching subsets, is there a way to use knowledge about which cells have changed in cand-syms since last subset search to optimize the combination-search?
  - Any combination composed only of cells whose number of cand-syms hasn't changed since the last find attempt can be skipped. (I think).
    - I'd be interested in doing some manual profiling to how often such a skip branch gets taken for order=5.