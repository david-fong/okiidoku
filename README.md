
# Sudoku Utilities in C++ and Rust

The project currently provides two utilities: a solution generator, and a canonicalizer. It is _not_ a solver or even a puzzle generator (hence "solve-n't"). It _is_ a solution generator and a canonicalizer (with perhaps more to be added in the future). It supports parametric size (which many other sudoku projects surpringly do not) while optimizing cache usage by choosing minimal value widths.

For usage instructions, see the readmes in the [cpp](./cpp/readme.md) and [rust](./rust/readme.md) directories.

I work on this project for fun. I want to come up with things without referencing other peoples' work. I am not at all an experienced C++ or rust programmer; I'm learning as I go. Feedback and suggestions on how to improve my usage of C++ and Rust are very welcome (but please be gentle if you can).

## But y u no solve tho

Other people have gone super technical into it and it blows me away while going way over my head. I'm not so interested in towing my chunky brain along a path that has already been expertly paved.

- [codegolf competition](https://codegolf.stackexchange.com/questions/190727/the-fastest-sudoku-solver)
  - [the winner, tdoku](https://t-dillon.github.io/tdoku/)
- [Emerentius/sudoku](https://github.com/Emerentius/sudoku)
- [Peter Norvig](https://norvig.com/sudoku.html)

## My Past and Future Related Works

Sudoku is a favourite puzzle game of mine. When I started programming in my first year of university, I wrote a solution generator in C. Looking back, I can say the code was terrible, but I still remember the feelings of excitement and gratification that I felt. I reworked it after learning Java (dusty private repo), then later started this. I've attempted a [hardware implementation using SystemVerilog](https://github.com/david-fong/Sudoku-SV), which so far has been a failure.
