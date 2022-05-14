# Why I Got Rid of GridArr and GridSpan

The following is a talk I had with myself to reason about whether it was actually good to have separate GridArr and GridSpan classes.

I'm beginning to question the value of having GridArr and GridSpan.

## Things Would Be Easier With Just GridArr

- No more complicated const-ness converting constructors (everything should be handled implicitly)
- No more CTAD guides for the converting constructors
- No worrying about how to do automatic change-dest-order-to-be-compatible-with-src for src-dest operations in the visitor interface.
  - This is a big one. I currently have no idea if I can address this when using the variant-of-spans abstraction, since I have no idea about the layout guarantees (if any) provided by the backing variant-of-array abstraction when using `std::variant`. I definitely haven't read anything on cppreference announcing something like that (though I haven't done any true scouring of the spec).

## Why Did I Do This In the First Place?

- Honestly I don't know if that was even a "good" idea.
- This goes back to when there was no mono/visitor distinction, and no custom container classes (which now are GridArr and GridSpan), and I was even using `std::array<std::arrayT, O2>, O2>` in some places.
- I had read a stack-overflow post about what span was and why it should be used and I just decided to go with that.

## Let's think about this

- We now have GridArr (before we didn't). Before, there was a more clear apparent case for why to use span: the user could pass array, vector, or anything with contiguous layout. That is- the library was relying on contiguous layout from its inputs. Anything with contiguous layout could have been fair game.

- Now we have GridArr, which is supposed to make the user's life easier by giving them (hopefully) the best choice of container at the most convenience. The existence of GridSpan seems to imply that other choices could exist, but I've never actually thought of whether that's the case. This library certainly doesn't offer anything such other choice. It currently has no reason to.

- Consider this: the original rationale for span was to also accept raw vector. Now that there's GridArr, there's not really a reason for using vector. Allowing users to easily use vector for each grid allows them to make the mistake of doing `vector<vector<>>` to create a vector of grids, which has the extra layer of indirection, which may be helpful when doing the vector data copy when resizing (just copy pointers instead of the backed data), but also doesn't give spatial locality between logically adjacent grids in the outer vector, and can also waste memory depending on the specific allocator used by the vector (allocators commonly have lists of different fixed block sizes). But with GridArr, (despite larger moves when resizing, which is preventable by using the capacity-control methods anyway), you get good locality and no entry-padding-due-to-allocator problems by default, which is much like the better choice that the user should have made (in the absence of GridArr), to use a flat vector instead (no inner vectors), which, though solving those problems, is a lower-level experience than I would really like to be putting on my users (which includes myself!). If the user wants that extra indirection for the resizing scenario and is okay with that allocator padding and loss of adjacent-entry locality, they can just use `vector<unique_ptr<GridArr>>`. GridArr is able to cover all original intended use-cases of `vector<Grid>` and removes lower-level details and mistake-cases from the user.The original argument for using `std::span` is rendered nil.

- If some user's project has its own grid containers that aren't compatible with my GridSpan (which I think would be likely, since GridSpan is a pretty strict layout contract: contiguous, specific-sized data), and they decide they don't want to change their stuff to meet GridSpan's requirements, and they can't just switch to using my GridArr (for some reason? I don't know why?), then they'd have to make conversions. Taking away my GridSpan class won't change the fact that they'd need to make conversions. In fact- the best way for them to provide a conversion with GridArr and GridSpan is to make a conversion to GridArr.

- If GridSpan didn't have compile-time bounds encoded in the type, and I didn't use custom entry types for each order, then vector would have been an easy way to get the same behaviour for containers as I now have by using `std::variant`. But those conditions aren't true. So `std::vector` wouldn't really have made things easier for me.

## I still feel uneasy about getting rid of GridSpan...

- For one thing, it wasn't trivial for me to implement. I got pushed to learn a lot of relatively-deeper C++ stuff. I guess I won some and lost some: I lost time, but learned how to do some interesting things that may possibly serve me well later (hopefully!).
- I have a feeling of- oh but maybe I should keep it for a while just in case. It would really suck to get rid of it and then later find that there's actually a good reason to have it. I estimate it would take a day or two just to add it back.
- But... If I don't currently have a reason for it, the idealist in me is telling me to just do it. To that idealist, the arguments presented above mean that GridSpan is just code clutter (and not a small amount of it).