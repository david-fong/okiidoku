
# Algorithms

## Equivalence Checking

Two Sudokus are equivalent if some combination of rule-following transformations which do not change the interrelationships of symbols in the Sudokus can cause them to become identical. Such transformations include shuffling rows/columns within row/column groups, shuffling row/column groups, mirroring along a diagonal, and shuffling symbols (ie. numbers. ex. replacing all ones with twos and vice versa). Note that rotation is included implicitly, since it can be performed by combinations of the other transformations.

Other people have also written about this topic, but focusing mainly on _counting_ equivalent and non-equivalent Sudokus. I didn't read any before drafting the following parts of this writing, so the terminology I use later is not the same as the community standard.

- [wikipedia.org](https://en.wikipedia.org/wiki/Mathematics_of_Sudoku#Essentially_different_solutions)
- [pi.math.cornell.edu](https://pi.math.cornell.edu/~mec/Summer2009/Mahmood/Symmetry.html).
- [handwiki.org](https://handwiki.org/wiki/Mathematics_of_Sudoku#Sudokus_of_other_sizes)

There are relatively fewer discussions/implementations concerning checking for equivalence:

- [this math stack exchange post](https://math.stackexchange.com/questions/1903186/determining-if-two-sudoku-boards-are-in-the-same-equivalence-class/4135784#4135784)
- [this math stack exchange post](https://math.stackexchange.com/questions/2791524/group-theory-and-sudoko)

Discussions related to equivalence checking can be found by searching for "Sudoku Canonicalization". Equivalence can be checked by canonicalizing two sudokus and then checking if the results are identical.

- [sudopedia.enjoysudoku.com](http://sudopedia.enjoysudoku.com/Canonical_Form.html)
- [dobrichev/sudoku-minlexing-tool](https://github.com/dobrichev/sudoku-minlexing-tool)
- [The rust Sudoku library](https://github.com/Emerentius/sudoku/issues/5)

### Key Properties of Transforms

Let's define an "atom" as an `O x 1` slice that fits inside a block group. The whole grid can be divided into `A / O` atoms all oriented either horizontally or vertically (they are two different "views"/"slicings").

- No transform ever trades symbols between atoms.
- No transform ever trades atoms between groups.

### Without Symbol Shuffling

Without symbol shuffling, equivalence is met if:

- (with or without a horizontal-vertical switch,)
- for every group of each kind (row, column, block) in one Sudoku, there is an equivalent group of the same kind in the other Sudoku (which has not yet been used in a previous group-equivalence matching)-
- where group equivalence is met when for each atom in one group, there is an equivalent atom in the other group-
- where atom equivalence is met when for each symbol in one atom, there is an instance of the same symbol in the other atom.

The above definition sufficiently accounts for all non-symbol-shuffling transformation-differences between two equivalent Sudokus, and it has much better time-domain behaviour than a blind brute-force search.

Let's nickname this check the "Post-Atom-Normalization Interrelation Comparison" ("PANIC").

### Normalizing Symbol Shuffling

By "normalizing symbol shuffling", I mean performing symbol shuffling transforms on two sudokus that- if they are equivalent- result in being able to prove equivalence by performing non-symbol-shuffling transforms that cause the two Sudokus to become visually identical (at which point we can use PANIC).

For each of the Sudokus- assuming the symbols are already mapped to numbers- for each Sudoku, do the following:

- Create a table with a row and column for each number-mapped symbol, where the index of the row/column is the value of the symbol.
- For each horizontal and vertical atom in the Sudoku,
  - For each unique choice of two different symbols from the atom,
    - Increment the value at the two coordinates in the table that correspond to that pair of symbols.
- Observe that choosing two different symbols and then swapping the two rows and then swapping the two columns that correspond to those symbols results in the table that would be created from a Sudoku where those two symbols were swapped.
  - Also observe that the sum of the counts in a row does not change after doing the above kind of shuffle.
- Sort the rows by the sum of their contained values, breaking ties according to the ordered values within the rows sorted by value. A symbol's normalized value is the index of its new row.

## Solution Generation
