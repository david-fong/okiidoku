# WASM and JavaScript Bindings

## Required Tools

- [the Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
- CMake 3.21 or greater
- A C/C++ build system. Recommended: Ninja

## Build Commands

```sh
# generate build-system configuration files
# cd to :/cpp/extras/bindings/emscripten
emcmake cmake -S . -B build/ -G 'Ninja Multi-Config' -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE

# build okiidoku
# cd build
cmake --build . --config=Release

# look at the symbol maps for fun
emnm okiidoku/Release/libokiidoku.a
emnm Release/libokiidoku_emscripten.a
```