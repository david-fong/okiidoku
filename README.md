# ōkiidoku

A C++ library for variable-grid-size sudoku to:

- generate random, full grids
- generate puzzles
- find all solutions to a puzzle using logic and brute-force
- (WIP) efficiently(?) put full grids in a canonical form
- (WIP) archive collections of grids in compressed form

It supports parametric grid size of dimensions 9x9, 16x16, 25x25, ..., 256x256, etc. and uses template meta-programming to allow optimizing for each case.

In the future, it will have language bindings for WASM/JS and Python.

programming-language-independent design notes can be found in the [design folder](./writings/design/). These are not specs or usage documentation. They are collections of the goals I have and the considerations I made in designing the containers and algorithms for the library.

In Japanese, "ookii" (大きい) means "big". I chose the name because one of my priorities for this project is to support variable grid sizes, and to find and engineer (ie. cache and memory efficiency) algorithms that scale well with large grids. Also, I thought the name had a nice ring- like "okey-dokey". I decided not to call the project "ookiidoku" (despite that being the proper romanization) out of fear of english speakers like myself mispronouncing it like [this](https://en.wikipedia.org/wiki/Close_back_rounded_vowel) instead of like [this](https://en.wikipedia.org/wiki/Mid_back_rounded_vowel).

I work on this project for fun. I like sudoku. I want to learn how to develop high-performance software libraries. I like reasoning about trade-offs. The algorithm design is like the puzzle to me; I always want to try to come up with things on my own before looking at how other people do things. Perhaps in doing so, I will come up with something new, and maybe even something that is better in some ways than existing approaches.

Here is [the roadmap](./cpp/TODO.md#roadmap) (subject to violent rearrangements at the mercy of my whim and fancy).

## Non-Goals / Non-Priorities

- Being on par with the fastest regular 9x9 sudoku solvers (tdoku, jczsolve, etc.).
  - With my limited time and knowledge, my higher priority is supporting variable grid size and keeping the code reasonably portable.
- Being a puzzle rater / implementing many deductive techniques.
  - Despite the first bullet, I am more interested in throughput than deductive power.
- Making explicit use of SIMD instructions.
  - My higher priority is keeping the code portable and somewhat malleable. Compilers and C++ standard library implementations can often infer and generate parallelized code, and for now, that's plenty good enough for me.
- Supporting sudoku variants other than kudoku.
  - You are welcome to fork and do such work separately.

## Usage

[See the readme in the cpp directory](./cpp/readme.md).

This project is MIT licensed.

If you have a question about the project and whether it is suitable for your own goals, please first take a look at [other existing projects](#other-existing-projects-and-solvers) and what they provide, and read through this project's [design docs](./writings/design/) before asking.

If you do something cool with it, that's great! I'd love to hear about it.

### Other Existing Sudoku Programs

- [t-dillon/tdoku](https://t-dillon.github.io/tdoku/)
- [jczsolve](http://forum.enjoysudoku.com/3-77us-solver-2-8g-cpu-testcase-17sodoku-t30470-210.html#p249309)
- [Emerentius/sudoku](https://github.com/Emerentius/sudoku)
- [Peter Norvig](https://norvig.com/sudoku.html)
- There are more. Google it.
  - [github:sudoku-generator](https://github.com/topics/sudoku-generator), [github:sudoku-puzzle](https://github.com/topics/sudoku-puzzle)
  - [sudopedia programs list](http://sudopedia.enjoysudoku.com/Sudoku_Programs.html)
  - [repos I've starred on github](https://github.com/stars/david-fong/lists/sudoku)

## My Past and Future Related Works

Sudoku is a favourite puzzle game of mine. After I took an intro to programming course in my first year of university, one of [my first personal projects](https://github.com/david-fong/my-first-projects) was a solution generator in C. I still remember the feelings of excitement and enjoyment that I felt. I reworked it after learning Java (dead private repo), then later started this. I've attempted a [hardware implementation using SystemVerilog](https://github.com/david-fong/Sudoku-SV), which so far has been a failure (probably due to too many wires).
