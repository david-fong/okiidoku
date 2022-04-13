# ōkiidoku

The project currently provides two utilities: a _solution_ generator, and a canonicalizer. It is _not_ a solver or even a _puzzle_ generator, although I will likely try my hand at those in the future. It supports parametric grid size which many other sudoku projects do not. The generator uses a [stochastic search](https://en.wikipedia.org/wiki/Sudoku_solving_algorithms#Stochastic_search_/_optimization_methods) algorithm, which was found to scale much better than a backtracking algorithm for very large grid sizes. The canonicalizer is currently under progress; I am experimenting with a novel approach using intrinsic statistics/characteristics of grids.

The project is still in its early stages. It it not stable. If you are looking for something to use as part of your own project, this project is _not ready_ (despite there being some usage instructions). You can find [its roadmap here](./cpp/TODO.md). While I'd like to have cross-platform support and support for most popular compilers, it's not one of my priorities. I also prioritize using modern technologies over supporting older ones.

Language-implementation independent design notes can be found in the [design folder](./design/).

In Japanese, "ookii" means "big". I chose the name because one of my priorities for this project is to support variable grid sizes, and to find algorithms that scale well with increased size. Also, I thought the name had a nice ring- like "okey-dokey". I decided not to call the project "ookiidoku" (despite that being the proper romanization) out of fear of english speakers like myself mispronouncing it like a monkey sound.

I work on this project for fun. I like sudoku. I want to learn how to develop high-performance software libraries. I like reasoning about trade-offs. The algorithm design is like the puzzle to me; I don't want to "look at answers" until I have tried my best come up with my own. Perhaps in doing so, I will come up with something new, and maybe even something that is better in some ways than existing approaches.

## Usage

See the readme in the [cpp](./cpp/readme.md) directory.

The [rust](./rust/readme.md) implementation is a learning project for me, and is currently _not under development_.

## Other Existing Projects and Solvers

[\*relevant meme](https://i.kym-cdn.com/photos/images/newsfeed/001/596/781/3b9.png)

- [codegolf competition](https://codegolf.stackexchange.com/questions/190727/the-fastest-sudoku-solver)
  - [the winner, tdoku](https://t-dillon.github.io/tdoku/)
- [Emerentius/sudoku](https://github.com/Emerentius/sudoku)
- [Peter Norvig](https://norvig.com/sudoku.html)
- There are more. Google it.
  - [github:sudoku-generator](https://github.com/topics/sudoku-generator), [github:sudoku-puzzle](https://github.com/topics/sudoku-puzzle)
  - [sudopedia programs list](http://sudopedia.enjoysudoku.com/Sudoku_Programs.html)

## My Past and Future Related Works

Sudoku is a favourite puzzle game of mine. After I took an intro to programming course in my first year of university, one of [my first personal projects](https://github.com/david-fong/my-first-projects) was a solution generator in C. The code was terrible, but I still remember the feelings of excitement and enjoyment that I felt. I reworked it after learning Java (private repo), then later started this. I've attempted a [hardware implementation using SystemVerilog](https://github.com/david-fong/Sudoku-SV), which so far has been a failure (probably due to too many wires).
