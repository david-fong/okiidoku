# ōkiidoku

The project currently provides these utilities: a _solution_ generator, a canonicalizer, and database-like utilities for compressed storage of solution and puzzle grids. It is _not_ a solver or even a _puzzle_ generator, although I will likely try my hand at those in the future. It supports parametric grid size which many other sudoku projects do not. The generator uses a [stochastic search](https://en.wikipedia.org/wiki/Sudoku_solving_algorithms#Stochastic_search_/_optimization_methods) algorithm, which was found to scale much better than a backtracking algorithm for very large grid sizes. The canonicalizer is currently under progress; I am experimenting with a novel approach using intrinsic statistics/characteristics of grids.

Language-implementation independent design notes can be found in the [design folder](./design/). This is an artifact of me originally aiming to create both a C++ and Rust implementation.

In Japanese, "ookii" means "big". I chose the name because one of my priorities for this project is to support variable grid sizes, and to find algorithms that scale well with increased size. Also, I thought the name had a nice ring- like "okey-dokey". I decided not to call the project "ookiidoku" (despite that being the proper romanization) out of fear of english speakers like myself mispronouncing it like a monkey sound.

I work on this project for fun. I like sudoku. I want to learn how to develop high-performance software libraries. I like reasoning about trade-offs. The algorithm design is like the puzzle to me; I don't want to "look at answers" until I have tried my best come up with my own. Perhaps in doing so, I will come up with something new, and maybe even something that is better in some ways than existing approaches.

## Usage

See the readme in the [cpp](./cpp/readme.md) directory.

### Preface

This project is MIT licenced. If something does not work for you, you can always do your own work and adapt it for your own needs.

The project is still in its early stages. It it not stable. You can find [its roadmap here](./cpp/TODO.md). While I'd like to have cross-platform support and support for most popular compilers, it's not one of my priorities. I also prioritize using modern technologies over supporting older ones.

If you have a question about the project and whether it is suitable for your own goals, please do your own research about [other existing projects](#other-existing-projects-and-solvers) and what they provide, and read through this project's [design docs](./design/) before asking. That being said, I recognize that people need to begin somewhere with learning to use software and various toolchains (myself included!), so if you find yourself completely lost as to how to begin and you _really_ want to try playing with this project, please make a post in the GitHub discussions page and I'll see if I can help.

- If it works for you and you do something cool with it, that's great! I'd love to hear about it.
- If you find configuration issues that prevent you from building the project using recent versions of a popular toolchain and you know how to fix it and submit a pull request, I will happily look at it, likely merge it in, and be very grateful.
- If you have a suggestion of how the library could be improved and you present a case, I will engage in discussion with you. Where applicable, benchmarks would also be appreciated.

## Other Existing Projects and Solvers

- [codegolf competition](https://codegolf.stackexchange.com/questions/190727/the-fastest-sudoku-solver)
  - [the winner, tdoku](https://t-dillon.github.io/tdoku/)
- [Emerentius/sudoku](https://github.com/Emerentius/sudoku)
- [Peter Norvig](https://norvig.com/sudoku.html)
- There are more. Google it.
  - [github:sudoku-generator](https://github.com/topics/sudoku-generator), [github:sudoku-puzzle](https://github.com/topics/sudoku-puzzle)
  - [sudopedia programs list](http://sudopedia.enjoysudoku.com/Sudoku_Programs.html)

## My Past and Future Related Works

Sudoku is a favourite puzzle game of mine. After I took an intro to programming course in my first year of university, one of [my first personal projects](https://github.com/david-fong/my-first-projects) was a solution generator in C. The code was terrible, but I still remember the feelings of excitement and enjoyment that I felt. I reworked it after learning Java (private repo), then later started this. I've attempted a [hardware implementation using SystemVerilog](https://github.com/david-fong/Sudoku-SV), which so far has been a failure (probably due to too many wires).
