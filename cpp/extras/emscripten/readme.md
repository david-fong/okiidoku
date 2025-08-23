<!-- SPDX-FileCopyrightText: 2020 David Fong -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
<!-- cspell:dictionaries cpp-refined -->
# WASM and JavaScript Bindings

## Required Tools

- CMake 4.1 or higher
- [A C++ build system](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html). Recommended: Ninja
- [Emscripten](https://emscripten.org/docs/getting_started/downloads.html) 4.0.7 or higher (which comes with [compiler, and NodeJS for _building_](https://emscripten.org/docs/building_from_source/toolchain_what_is_needed.html))
- For running the output JS, Node 16 or higher.
  <!-- maintainer note: From `rg '^var MIN_NODE_VERSION\s?=\s\d+;' ${EMSDK}/upstream/emscripten/src/settings.js`. Reflect in package.json::engines -->

I have no intention to support versions of Emscripten older than the latest at the time of my last pull.

## Command Snippets

Note: The specific cmake configuration commands are just examples. You can do whatever works for you.

```sh
# generate build-system configuration files (run after cloning the repo)
# (cd to :/cpp)
export EMSDK=#<path to emsdk>
cmake -S . --preset=dev.emscripten

# build okiidoku (rerun after modifying the library)
cmake --build --preset=dev.emscripten.release

# look at the symbol maps for fun
# (cd to ./out/build/dev.emscripten)
emnm -nC --defined-only okiidoku/Release/libokiidoku.a | less

# cd into the build directory
node -i -e 'const oki = require(path.join(process.cwd(), "bin/okiidoku.js"))'
```

## Code Usage

See the [examples folder](./examples/).

Friendly reminder to [call `.delete()` when you are done with an object](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#memory-management) living in the WASM memory (unless that object was bound using `emscripten::value_object`). Unfortunately there are these things you (and I) will have to deal with in using this library. It just takes some learning of how things work, and being careful.

- https://developer.chrome.com/blog/wasm-debugging-2020
- https://developer.chrome.com/docs/devtools/wasm