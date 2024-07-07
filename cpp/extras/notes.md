<!-- SPDX-FileCopyrightText: 2020 David Fong -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
<!-- cspell:dictionaries cpp-refined -->
# Language Bindings

## Notes on SWIG

There is [SWIG](https://www.swig.org/exec.html), but from extremely shallow online searching,
- for python, compared to Boost.Python, C++ devs find it advantageous to use Boost.Python instead of SWIG for performance and fluency/idiom reasons ([link](https://news.ycombinator.com/item?id=14251821)).
- for the web, SWIG doesn't seem to support running the output in a browser ([link](https://github.com/swig/swig/issues/1808)).

## WASM and JS

- the cmake preset "dev" inherits the toolchain preset's condition. If [this](https://gitlab.kitware.com/cmake/cmake/-/issues/23283) gets implemented, we can make use of it to have a clean solution.

- emscripten links and things for future reference
  - `cmake -S . --preset dev.emscripten`
  - [conan and emscripten](https://docs.conan.io/en/latest/integrations/cross_platform/emscripten.html)
  - [compiling](https://emscripten.org/docs/compiling/WebAssembly.html)
  - [EmBind vs WebIDL Binder](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html#interacting-with-code-binding-cpp)
    - I'll use embind
    - [embind](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html)
  - [the JS module object](https://emscripten.org/docs/api_reference/module.html#module)
  - why aren't `compare_three_way_result` and things like `is_neq` found? I can clearly see them declared in emscripten/upstream/lib/c++/v1/compare

## Python

- Oh boy. There's a pybind11 successor: https://github.com/wjakob/nanobind

- [PyBind11](https://pybind11.readthedocs.io/en/stable/)
  - based on Boost.Python. Seems people think well of it and like it a bit more than Boost.Python.
  - Has the advantage that I don't need to pull in all of boost just to get this.

- [creating a setup.py](https://stackoverflow.com/a/73153215/11107541)

- [Boost.Python](https://www.boost.org/doc/libs/1_79_0/libs/python/doc/html/index.html)
  - [reducing compiling time](https://www.boost.org/doc/libs/1_50_0/libs/python/doc/tutorial/doc/html/python/techniques.html#python.reducing_compiling_time)

## Java

not yet confirmed to be on roadmap.

- There's JNI.
- Java 19 has something new [](https://openjdk.org/jeps/424)