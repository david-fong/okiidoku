# Things To Do

## Roadmap

- visitor pattern
- see where template deduction guides can make the mono interfaces nicer to use
  - draft up an examples folder for the lib with one for mono and one for visitor (will be very much like the tests)
- test CMake config with different compilers (gcc, clang, apple-clang, MSVC) and try to fix configuration issues
- draft database
- refactor and improve canonicalization
- compare backtracking and stochastic search statistics
- implement grid-serdes translator tool
- refine database
- create a separate github repo for data. add as submodule?
- deductive reasoning + backtracking solver
- emscripten build and website
- puzzle creation experiments (see [bottom section](#interesting-questions-for-further-research))
- minlexing canonicalization
- compare canonicalization methods (performance / time-complexity)
- opencl stochastic generator
  - some template/starter code can be found at commit 5a8d4c0 (deleted)

## Misc List

- TODO.high benchmark to see impact of always initializing `GridArr`. default constructor can currently leave the grid uninitialized (I think?). this is currently left as is for performance reasons, but I need to benchmark to see how justified doing this is.

- Brainstorm ways to have a strategy for the detail namespace(s). Currently there is no method to the madness.

- I'm beginning to question the value of having GridArr and GridSpan.
  - It seems like everything would be a lot easier if I only used GridArr:
    - No more complicated const-ness converting constructors (everything should be handled implicitly)
    - No more CTAD guides for the converting constructors
    - No worrying about how to do automatic change-dest-order-to-be-compatible-with-src for src-dest operations in the visitor interface.
      - This is a big one. I currently have no idea if I can address this when using the variant-of-spans abstraction, since I have no idea about the layout guarantees (if any) provided by the backing variant-of-array abstraction when using `std::variant`. I definitely haven't read anything on cppreference announcing something like that (though I haven't done any true scouring of the spec).
  - The reason why I originally made the array/span distinction:
    - Honestly I don't know if that was even a "good" idea.
    - This goes back to when there was no mono/visitor distinction, and no custom container classes (which now are GridArr and GridSpan), and I was even using `std::array<std::arrayT, O2>, O2>` in some places.
    - I had read a stack-overflow post about what span was and why it should be used and I just decided to go with that.
  - Let's think about this:
    - We now have GridArr (before we didn't). Before, there was a more clear apparent case for why to use span: the user could pass array, vector, or anything with contiguous layout. That is- the library was relying on contiguous layout from its inputs. Anything with contiguous layout could have been fair game.
    - Now we have GridArr, which is supposed to make the user's life easier by giving them (hopefully) the best choice of container at the most convenience. The existence of GridSpan seems to imply that other choices could exist, but I've never actually thought of whether that's the case. This library certainly doesn't offer anything such other choice. It currently has no reason to.
    - Consider this: the original rationale for span was to also accept raw vector. Now that there's GridArr, there's not really a reason for using vector. Allowing users to easily use vector for each grid allows them to make the mistake of doing `vector<vector<>>` to create a vector of grids, which has the extra layer of indirection, which may be helpful when doing the vector data copy when resizing (just copy pointers instead of the backed data), but also doesn't give spatial locality between logically adjacent grids in the outer vector, and can also waste memory depending on the specific allocator used by the vector (allocators commonly have lists of different fixed block sizes). But with GridArr, (despite larger moves when resizing, which is preventable by using the capacity-control methods anyway), you get good locality and no entry-padding-due-to-allocator problems by default, which is much like the better choice that the user should have made (in the absence of GridArr), to use a flat vector instead (no inner vectors), which, though solving those problems, is a lower-level experience than I would really like to be putting on my users (which includes myself!). If the user wants that extra indirection for the resizing scenario and is okay with that allocator padding and loss of adjacent-entry locality, they can just use `vector<unique_ptr<GridArr>>`. The mistake-case is eliminated, and better abstractions are the default. TLDR, GridArr makes good `vector<Grid>` easy, and the original argument for using `std::span` is rendered nil.
    - If some user's project has its own grid containers that aren't compatible with my GridSpan (which I think would be likely, since GridSpan is a pretty strict layout contract: contiguous, specific-sized data), and they decide they don't want to change their stuff to meet GridSpan's requirements, and they can't just switch to using my GridArr (for some reason? I don't know why?), then they'd have to make conversions. Taking away my GridSpan class won't change the fact that they'd need to make conversions. In fact- the best way for them to provide a conversion with GridArr and GridSpan is to make a conversion to GridArr.
    - If GridSpan didn't have compile-time bounds encoded in the type, and I didn't use custom entry types for each order, then vector would have been an easy way to get the same behaviour for containers as I now have by using `std::variant`. But those conditions aren't true. So `std::vector` wouldn't really have made things easier for me.
  - I still feel uneasy about getting rid of GridSpan...
    - For one thing, it wasn't trivial for me to implement. I got pushed to learn a lot of relatively-deeper C++ stuff. I guess I won some and lost some: I lost time, but learned how to do some interesting things that may possibly serve me well later (hopefully!).
    - I have a feeling of- oh but maybe I should keep it for a while just in case. It would really suck to get rid of it and then later find that there's actually a good reason to have it. I estimate it would take a day or two just to add it back.
    - But... If I don't currently have a reason for it, the idealist in me is telling me to just do it. To that idealist, the arguments presented above mean that GridSpan is just code clutter (and not a small amount of it).

- read about how dllexport works for MSVC and find out why the current usage is insufficient for class methods.

- create a custom BitsO2 class that attempts to use efficient x86 instructions when possible and falls back to something portable otherwise.
  - Most of the time in this library, there will not need to be a whole lot of these. For scanning a grid row-major-wise, that's 1+O1+O2 of them. If one is needed for each house, that's 3*O2 of them. Not sure if there would be significant gains from customizing storage to be more compact than u64 granularity...
  - Try to write clear and strict contracts to prevent having to handle excess bits (those that exist higher than the bits that are actually required).
    - The bit flip things are particularly problematic. Currently only used as optimized ways to resetting bits. Could they be removed? The shift operators I think are also only there because I didn't add set/reset methods. Hopefully most of those things can be replaced with just set/reset.
  - The header and implementation could actually be private. Would we want to expose it to users of the library?

- change the size-variable versions of the library functions to use the visitor pattern? Need to create a common variant grid class.
  - [it is allowed in c++ to have template functions and non-template functions with the same name. here are the resulting rules](https://stackoverflow.com/a/16865452/11107541). Could this be used to put the algorithm functions under the same namespace (not in separate "mono" and "visitor" namespace)?

- Consider a (multi-threaded) batch api that gives the caller control over the logic for when to stop.

- move the emoji definitions out of the program binary and externalize as a configurable data read in at runtime?

- what's this?
  - https://stackoverflow.com/questions/4977252/why-an-unnamed-namespace-is-a-superior-alternative-to-static

- cmake things I can try out:
  - versioning my targets https://cmake.org/cmake/help/latest/prop_tgt/VERSION.html#prop_tgt:VERSION
    - cppcon video <https://www.youtube.com/watch?v=m0DwB4OvDXk&ab_channel=CppCon&t=17m13s>

- try adding shortcuts to std::views::iota(0,N) and use them instead of all the raw loops. see how it impacts performance. Don't know if compilers will still do all their optimizations (simd, unrolling, etc.).

- Note to self: the visitor interface of the library could theoretically be made such that separate dynamic libraries containing multiple non-overlapping compiled orders could be linked together via the visitor interface.
  - would require making the visitor stuff a separate library and giving it a tweak config with all the expected compiled orders to get from the mono libraries.
  - I currently don't see any benefit from this. And as I found earlier when trying this out, I didn't like how it made the repo folder tree much bigger and (slightly- enough to be quite annoying) harder to navigate.

- Go back and try the old canonicalization by rel row prob, but break ties by doing some brute force: try each tied permutation and valuate it according to some reduction of how it pushes rarer rel counts to the top left. Just be careful to shift to get rid of the main diagonal seam.
  - If there are multiple puddles of ties, the resolution of a puddle shouldn't depend on the resolution of any other puddle- only on the non-tied rows/columns. A consequence of this is that this resolution algorithm will not work if there are no non-tied rows/columns.

1. Consider: The current relabelling canonicalization method may have a big weakness: I think ties can be easily crafted: consider the "Most Canonical" solution grid- it would be all ties. How can this be addressed? (Or perhaps the "Most Canonical" grid is the only weakness?)
    - Break ties by designing a way to give symbols that frequently cohabit atoms label-values that are closer together in value:
    - The cohabitation table can be tiled; a coordinate's vertical or horizontal distance (these are the same, since relabelling moves both the col and row together) from the closest tiling of the main diagonal represents the distance of the labels from each other.
    - For each label, make an array where each index represents another label, and the value is an object containing the left and right distances between them, and the cohabitation count.
    - Hm. So far this seems to suggest a hill-climbing / brute-force-type solution...
    - OR... make an empty commit saying that a solution was implemented using the [Ostrich Algorithm](https://en.wikipedia.org/wiki/Ostrich_algorithm)

- ? Refactor names to use terminology suitable for more than just 2 dimensions? Ex. in 2D: row -> `d0i` (as in "dimension-zero index"), col -> `d1i`. But doing so would imply that I'm going to support multiple dimensions... and that's a huge can of worms.

- Decide what interfaces to support:
  - Probably best to start with just readline and a CLI
    - For CLI util libraries, look into using
      - repl
        - https://github.com/daniele77/cli
      - command
        - https://github.com/docopt/docopt.cpp
        - https://github.com/CLIUtils/CLI11
        - http://tclap.sourceforge.net/manual.html
        - https://github.com/Taywee/args
  - Can look into ncurses in the future? Or look into options for TUI libraries?
  - A web interface would be really nice.
- C++20
  - `using enum`. Might want to wait for CLANG to support?
- C++23
  - http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2214r0.html#does-adjacent-mean-2-or-n
  - `std::unreachable()`
  - `std::ranges::iota()`
  - `std::views::chunk()`
    - could this be useful for `canon_place`? I tried messing around with range-v3 on compiler explorer and had trouble with the whole action vs algorithm, container vs view thing. Didn't know what I was doing and could achieve what I wanted.
  - alternative to `std::chunk`, look into `mdspan` (multi-dimensional span). Seems like this is more of what I'm looking for.
  - multidimensional subscript operator
- CLI
  - implement `-h` and `--help` CLI argument.
  - give a red message when trying to continue and nothing is left to be found.
- do I want to generate doxygen docs?
  - https://jothepro.github.io/doxygen-awesome-css/index.html