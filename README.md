
# Sudoku Solution Generation with C++

While most existing programming work related to Sudoku tackles the problem of _solving_ puzzles, this one looks at _generating_ the solutions that are then turned into puzzles. The goal is to answer the question, "How can we generate a highly random Sudoku solution (a full and valid Sudoku grid) out of nothing, and do so within a short timeframe for largeish grid sizes (ie. not just 9x9)." The statement is not incredibly precise, but that's because there isn't a lot of other work to benchmark against.

I have made a simple, web-based visualizer of the algorithm at work in a sister repo, [**Sudoku-JS**](https://david-fong.github.io/Sudoku-JS/), which I highly encourage you to take a look at.

## How to Run It

Make sure you have g++ installed.

Note: If your terminal doesn't support ANSI escape codes for colored text, set [`USE_ANS_ESC` in `buildflag.hpp`](src/buildflag.hpp) to `false`.

```shell
cd src
./build
./main
```

<img src="images/25x25_example.png" width="44%" align="left" />

## My Past and Future Related Works

Sudoku is a favourite puzzle game of mine. When I started programming in my first year of university, I wrote a solution generator in C. Looking back, I can say the code was terrible, but I still remember the feelings of excitement and gratification that I felt. I [reworked it after learning Java](https://github.com/david-fong/Sudoku-J), then later started this repo, and next, I look to write [a hardware implementation using SystemVerilog](https://github.com/david-fong/Sudoku-SV).

