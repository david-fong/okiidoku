# Finding Subsets

Date: 2022/June/13

Did some comparison for finding subsets. Used the puzzle maker with order 5, and waited until 249 puzcell candidates remained. For no subset finding, it took 2m48s. For full subset finding with empty guess stack, it took 2m28s. For finding subsets of size 2 regardless of guess stack depth, it took 1m48s.

For effort parameterization, what's a good way to do it?

- Take a max-subset-size param to dictate up to what subset size of either type to try to find.
  - This is nice and simple to implement, but there's an alternative that has closer correspondence to constraining the "combinatorial explosion": If the subset that we are trying to partition by finding a sub-subset in it is very close in size to the naked subset size being searched, then the number of combinations is also relatively small.
- Take a param to dictate how many combinations to try.
  - Also simple to implement (on the callee side) and can correspond closer to constraining combinatorial explosion.
- Take a param P to dictate effort, where for N less than O2, it will search for subsets of up to the size K where the sum of the number of combinations it would have to try over all those subset sizes in the worst case would be less than or equal to the number of combinations if N was O2 and K was P.