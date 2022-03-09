
# Sudoku Utilities in C++

The project currently provides two utilities: a _solution_ generator, and a canonicalizer. It is _not_ a solver or even a _puzzle_ generator (hence "solve-_n't_"), although I may try my hand at those in the future. It supports parametric grid size which many other sudoku projects do not.

For usage instructions, see the readmes in the [cpp](./cpp/readme.md) and [rust](./rust/readme.md) directories. Note that the rust implementation is more of a learning project for me, and is currently not under development.

I work on this project for fun. I like sudoku. I want to learn how to develop high-performance software libraries. I like reasoning about trade-offs. I don't want to copy other peoples' algorithms; the algorithm design is like the puzzle to me; I don't want to "look at answers" at least until I have come up with my own. Perhaps in doing so, I will come up with something new, and maybe even something that is better in some ways than existing approaches. Regardless, I will do my best :)

Feedback and suggestions on how to improve are very welcome- particularly about best practice in general software development / library authoring with C++ or rust. Just be prepared to help me understand the rationale and how things work. I will appreciate it.

## If You Want a Solver

[\*relevant meme](https://i.kym-cdn.com/photos/images/newsfeed/001/596/781/3b9.png)

- [codegolf competition](https://codegolf.stackexchange.com/questions/190727/the-fastest-sudoku-solver)
  - [the winner, tdoku](https://t-dillon.github.io/tdoku/)
- [Emerentius/sudoku](https://github.com/Emerentius/sudoku)
- [Peter Norvig](https://norvig.com/sudoku.html)
- There are more. Google it.
  - [github:sudoku-generator](https://github.com/topics/sudoku-generator), [github:sudoku-puzzle](https://github.com/topics/sudoku-puzzle)

## My Past and Future Related Works

Sudoku is a favourite puzzle game of mine. After I took an intro to programming course in my first year of university, one of my first personal projects was a solution generator in C. The code was terrible, but I still remember the feelings of excitement and enjoyment that I felt. I reworked it after learning Java (dusty private repo), then later started this. I've attempted a [hardware implementation using SystemVerilog](https://github.com/david-fong/Sudoku-SV), which so far has been a failure.
