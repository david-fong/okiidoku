# ōkiidoku

The project can:

- generate random, full grids
- generate puzzles
- solve puzzles using logic and brute-force
- (WIP) efficiently(?) put full grids in a canonical form
- (WIP) archive collections of grids in highly-compressed form

It supports parametric grid size in a performant manner. In the future, it will have language bindings for WASM/JS and Python.

Language-implementation independent design notes can be found in the [design folder](./writings/design/). These are not specs or usage documentation. They are collections of the goals I have and the considerations I made in designing the containers and algorithms for the library.

In Japanese, "ookii" means "big". I chose the name because one of my priorities for this project is to support variable grid sizes, and to find and engineer (ie. cache and memory efficiency) algorithms that scale well with large grids. Also, I thought the name had a nice ring- like "okey-dokey". I decided not to call the project "ookiidoku" (despite that being the proper romanization) out of fear of english speakers like myself mispronouncing it like a monkey sound.

I work on this project for fun. I like sudoku. I want to learn how to develop high-performance software libraries. I like reasoning about trade-offs. The algorithm design is like the puzzle to me; I don't want to "look at answers" until I have tried my best come up with my own. Perhaps in doing so, I will come up with something new, and maybe even something that is better in some ways than existing approaches.

Here is [the roadmap](./cpp/TODO.md) (subject to violent rearrangements at the mercy of my whim and fancy).

## Usage

See the readme in the [cpp](./cpp/readme.md) directory.

This project is MIT licensed.

If you have a question about the project and whether it is suitable for your own goals, please do your own research about [other existing projects](#other-existing-projects-and-solvers) and what they provide, and read through this project's [design docs](./writings/design/) _before_ asking.

If you do something cool with it, that's great! I'd love to hear about it.

### Some Other Existing Projects and Solvers

- [codegolf competition](https://codegolf.stackexchange.com/questions/190727/the-fastest-sudoku-solver)
  - [the winner, tdoku](https://t-dillon.github.io/tdoku/)
- [Emerentius/sudoku](https://github.com/Emerentius/sudoku)
- [Peter Norvig](https://norvig.com/sudoku.html)
- There are more. Google it.
  - [github:sudoku-generator](https://github.com/topics/sudoku-generator), [github:sudoku-puzzle](https://github.com/topics/sudoku-puzzle)
  - [sudopedia programs list](http://sudopedia.enjoysudoku.com/Sudoku_Programs.html)

## My Past and Future Related Works

Sudoku is a favourite puzzle game of mine. After I took an intro to programming course in my first year of university, one of [my first personal projects](https://github.com/david-fong/my-first-projects) was a solution generator in C. I still remember the feelings of excitement and enjoyment that I felt. I reworked it after learning Java (dead private repo), then later started this. I've attempted a [hardware implementation using SystemVerilog](https://github.com/david-fong/Sudoku-SV), which so far has been a failure (probably due to too many wires).
