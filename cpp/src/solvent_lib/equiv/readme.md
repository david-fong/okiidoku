# Equivalence

## Background

Two Sudokus are equivalent if some combination of rule-following transformations which do not change the interrelationships of symbols in the Sudokus can cause them to become identical. Such transformations include shuffling rows/columns within row/column groups, shuffling row/column groups, mirroring along a diagonal, and shuffling symbols (ie. numbers. ex. replacing all ones with twos and vice versa). Note that rotation is included implicitly, since it can be performed by combinations of the other transformations.

Other people have also written about this topic, but focusing mainly on _counting_ equivalent and non-equivalent Sudokus. I didn't read any before drafting the following parts of this writing, so the terminology I use later is not the same as the community standard.

- [wikipedia.org](https://en.wikipedia.org/wiki/Mathematics_of_Sudoku#Essentially_different_solutions)
- [pi.math.cornell.edu](https://pi.math.cornell.edu/~mec/Summer2009/Mahmood/Symmetry.html).
- [handwiki.org](https://handwiki.org/wiki/Mathematics_of_Sudoku#Sudokus_of_other_sizes)

There isn't an abundance of community discussion on the topic of equivalence checking:

- [this math stack exchange post](https://math.stackexchange.com/questions/1903186/determining-if-two-sudoku-boards-are-in-the-same-equivalence-class/4135784#4135784)
- [this math stack exchange post](https://math.stackexchange.com/questions/2791524/group-theory-and-sudoko)

Discussions related to equivalence checking can be found by searching for "Sudoku Canonicalization". Equivalence can be checked by canonicalizing two sudokus and then checking if the results are identical.

- [sudopedia.enjoysudoku.com](http://sudopedia.enjoysudoku.com/Canonical_Form.html)
- [dobrichev/sudoku-minlexing-tool](https://github.com/dobrichev/sudoku-minlexing-tool)
- [The rust Sudoku library](https://github.com/Emerentius/sudoku/blob/master/src/board/canonicalization.rs)
- https://www.degruyter.com/document/doi/10.2478/s13537-012-0011-y/pdf
- https://sudokugarden.de/en/info/canonical-form

## Unscramble-able Relationships

Scrambling cannot:
- Swap cells between 1xO1 (vertical or horizontal) block-aligned slices (I will call this an "atom").
- Swap cells between houses.
- Swap blocks between chutes.
- Change the vertical/horizontal orientation of an atom without doing so for all atoms (transposing the grid).

## My Canonicalization Algorithm

Notable properties:

- no "brute forcing" / "guess-and-check".
- default to factoring as many cells as possible into the sorting bases, and only factoring them out when necessary to break ties instead of vice versa.
- TODO what is the time complexity?

Note that there are many possible variations on the calculations / measures used in my algorithm that can just as well canonicalize. I made decisions according to the above goals.

The end result of the specific choices made is that:

- Labels are given such that lower valued labels "play favourites". (they will cohabit atoms with some label values more than others).
- Instances of favouritism will be more concentrated at the top and left of the grid.

This may help with solving- labelling and positioning are "sorted" to specific relationship patterns.