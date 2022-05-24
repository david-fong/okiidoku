# WASM and JavaScript Bindings

## Required Tools

- [the Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
- CMake 3.21 or greater
- A C/C++ build system. Recommended: Ninja

## Build Commands

```sh
# generate build-system configuration files
# (cd to :/cpp/extras/bindings/emscripten)
emcmake cmake -S . -B build/ -G 'Ninja Multi-Config' -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE

# build okiidoku
# (cd to ./build)
cmake --build . --config=Release

# look at the symbol maps for fun
emnm -nC --defined-only okiidoku/Release/libokiidoku.a | less
```

## Code Usage

In the future, I will populate the examples folder with an example for the web and for nodejs.

Friendly reminder to [call `.delete()` when you are done with an object](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#memory-management) (unless that object was bound using `emscripten::value_object`). Unfortunately there are these things you (and I) will have to deal with in using this library. It just takes some learning of how things work, and being careful.