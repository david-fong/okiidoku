<!-- SPDX-FileCopyrightText: 2020 David Fong -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
# ōkiidoku

A C++ library for variable-grid-size sudoku to:

- generate random, full grids
- (WIP) generate puzzles
- find all solutions to a puzzle using logic and brute-force
- (WIP) efficiently(?) put full grids in a canonical form
- (WIP) archive collections of grids in compressed form

It supports parametric grid size of dimensions 9x9, 16x16, 25x25, ..., 256x256, etc. and uses template meta-programming to allow optimizing for each case.

In the future, it will have language bindings for WASM/JS and Python.

programming-language-independent design notes can be found in the [design folder](./writings/design/). They go into more detail about the goals I have and the considerations I made in designing the containers and algorithms for the library.

Here is [the roadmap](./cpp/TODO.md#roadmap) (subject to violent rearrangements at the mercy of my whim and fancy).

## Goals and Priorities

In order:

1. Supporting variable grid size (`N^2 x N^2`)
1. Portability (target architectures and recent compilers)
1. (ordering not as clear-cut):
   1. Readability and Maintainability
   1. Speed / Throughput
   1. Memory consumption

### Non-Goals / Non-Priorities

- Being on par with the fastest regular 9x9 sudoku solvers (tdoku, jczsolve, etc.).
- Being a puzzle rater / implementing many deductive techniques.
- Making explicit use of SIMD instructions.
  - Portability takes priority. Compilers and C++ standard library implementations do a pretty good job of generating vectorized code.
- Supporting sudoku variants other than sukaku (pencilmark sudoku).
  - I'm open to supporting variants which can be implemented non-intrusively as a layer on top of the existing machinery.

## Usage

[See the readme in the cpp directory](./cpp/readme.md).

For Python and WASM/JS bindings, see the [extras folder](./cpp/extras/)

This project is licensed under the [GNU Affero General Public License or (at your option) any later version](./LICENSES/AGPL-3.0-or-later-3.0-or-later.txt).

If you have a question about the project and whether it is suitable for your own goals, please first take a look at other existing projects and what they provide, and read through this project's [design docs](./writings/design/) before asking. Use the [discussion page](https://github.com/david-fong/okiidoku/discussions) for questions rather than raising a GitHub issue.

If you do something cool with it, that's great! I'd love to hear about it.

## Other Existing Sudoku Programs

- Speed-focused:
  - [t-dillon/tdoku](https://t-dillon.github.io/tdoku/)
  - [jczsolve](http://forum.enjoysudoku.com/3-77us-solver-2-8g-cpu-testcase-17sodoku-t30470-210.html#p249309)
  - [dobrichev/fsss2](https://github.com/dobrichev/fsss2)
  - [Emerentius/sudoku](https://github.com/Emerentius/sudoku)
- Deduction-focused:
  - [SudokuMonster/SukakuExplainer](https://github.com/SudokuMonster/SukakuExplainer)
- Some more lists:
  - [github:sudoku-generator](https://github.com/topics/sudoku-generator), [github:sudoku-puzzle](https://github.com/topics/sudoku-puzzle)
  - [sudopedia programs list](http://sudopedia.enjoysudoku.com/Sudoku_Programs.html)
  - [repos I've starred on github](https://github.com/stars/david-fong/lists/sudoku)

## The Meaning of the Name

In Japanese, "ookii" (大きい) means "big". I chose the name because one of my priorities for this project is to support variable grid sizes, and to find and engineer (ie. cache and memory efficiency) algorithms that scale well with large grids. Also, I thought the name had a nice ring- like "okey-dokey". I decided not to call the project "ookiidoku" (despite that being the proper romanization) out of fear of english speakers like myself mispronouncing it like [this](https://wikipedia.org/wiki/Close_back_rounded_vowel) instead of like [this](https://wikipedia.org/wiki/Mid_back_rounded_vowel).

## Personal Motivation

Sudoku is a favourite puzzle game of mine. After I took an intro to programming course in my first year of university, one of [my first personal projects](https://github.com/david-fong/my-first-projects) was a solution generator in C. I still remember the feelings of excitement and enjoyment that I felt. I reworked it after learning Java (dead private repo), then later started this. I've attempted a [hardware implementation using SystemVerilog](https://github.com/david-fong/Sudoku-SV), which so far has been a failure (probably due to too many wires).

I work on this project for fun. I want to learn how to develop high-performance software libraries. I like reasoning about trade-offs. The algorithm design is like the puzzle to me; I always want to try to come up with things on my own before looking at how other people do things. Perhaps in doing so, I will come up with something new, and maybe even something that is better in some ways than existing approaches.
