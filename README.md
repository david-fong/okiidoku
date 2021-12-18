
# Sudoku Utilities in C++ and Rust

The project currently provides two utilities: a solution generator, and a canonicalizer. It is _not_ a solver or even a puzzle generator (hence "solve-n't"), although I may try tackling these problems in the future. It supports parametric size which many other sudoku projects do not.

For usage instructions, see the readmes in the [cpp](./cpp/readme.md) and [rust](./rust/readme.md) directories. Note that the rust implementation is more of a learning project for me, and is currently not in development.

I work on this project for fun. Feedback and suggestions on how to improve are very welcome- just be prepared to help me understand the rationale.

## If You Want a Solver

\*Insert image of spongebob indignantly gesturing to many instances of a thing here.

- [codegolf competition](https://codegolf.stackexchange.com/questions/190727/the-fastest-sudoku-solver)
  - [the winner, tdoku](https://t-dillon.github.io/tdoku/)
- [Emerentius/sudoku](https://github.com/Emerentius/sudoku)
- [Peter Norvig](https://norvig.com/sudoku.html)
- There are more. Google it.

## My Past and Future Related Works

Sudoku is a favourite puzzle game of mine. When I started programming in my first year of university, I wrote a solution generator in C. Looking back, I can say the code was terrible, but I still remember the feelings of excitement and gratification that I felt. I reworked it after learning Java (dusty private repo), then later started this. I've attempted a [hardware implementation using SystemVerilog](https://github.com/david-fong/Sudoku-SV), which so far has been a failure.
