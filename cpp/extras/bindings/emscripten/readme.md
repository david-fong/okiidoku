# WASM and JavaScript Bindings

[See here for instructions on installing the Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html).

```sh
# run this for first-time setup / upon changing CMake configuration
emcmake cmake -S . -B build/ -G 'Ninja Multi-Config'

# run this to build/rebuild
cd build
cmake --build . --config=Release
```