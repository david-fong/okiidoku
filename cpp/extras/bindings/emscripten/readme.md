<!-- SPDX-FileCopyrightText: 2020 David Fong -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
<!-- cspell:dictionaries cpp-refined -->
# WASM and JavaScript Bindings

## Required Tools

- [Emscripten](https://emscripten.org/docs/getting_started/downloads.html)
- CMake 3.22 or later
- A C/C++ build system. Recommended: Ninja

I have no intention to support versions of Emscripten older than the latest.

## Command Snippets

Note: The specific cmake configuration commands are just examples. You can do whatever works for you.

```sh
# generate build-system configuration files (run after cloning the repo)
# (cd to :/cpp)
export EMSDK=#<path to emsdk>
cmake -S . --preset=dev.emscripten

# build okiidoku (rerun after modifying the library)
# (cd to ./build)
cmake --build . --config=Release

# look at the symbol maps for fun
emnm -nC --defined-only okiidoku/Release/libokiidoku.a | less

node const oki = require(path.join(process.cwd(), "Release/okiidoku.js"))
```

## Code Usage

See the [examples folder](./examples/).

Friendly reminder to [call `.delete()` when you are done with an object](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#memory-management) living in the WASM memory (unless that object was bound using `emscripten::value_object`). Unfortunately there are these things you (and I) will have to deal with in using this library. It just takes some learning of how things work, and being careful.