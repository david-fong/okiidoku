
# C++ Okiidoku

## Using the Dynamic Library

Building okiidoku requires CMake (with [the minimum specified version](./CMakeLists.txt)), a C++ compiler [supporting C++20](https://en.cppreference.com/w/cpp/compiler_support), and a build system like Make or Ninja. Current gcc and MSVC are supported; Clang support is close. Read the [TODO file](./TODO.md) for more info.

Refer to the [CMake guide](https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html). This may also help: [tldp.org on shared libraries](https://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html)

- The library uses templates for each compiled grid order for optimization purposes. The templates are accessible under the `okiidoku::mono` namespace, and a visitor-pattern interface is exposed under the `okiidoku::visitor` namespace. The visitor pattern is intended to make it more convenient to _write_ code that uses an order selected at runtime.
  - Code examples can be found in [the examples folder](./libs/okiidoku/examples/).

- Make sure to read and follow the api contracts documented in the headers. Exceptions are intentionally not thrown for precondition violations. If you would like to run against a build of libokiidoku with assertions for debugging purposes, create and use a build of libokiidoku with `-DCMAKE_BUILD_TYPE=Debug`.

- To change the supported grid sizes that get compiled, create a [tweak header](https://vector-of-bool.github.io/2020/10/04/lib-configuration.html#providing-a-tweak-header) for [`./libs/okiidoku/include/okiidoku/config/defaults.hpp`](./libs/okiidoku/include/okiidoku/config/defaults.hpp).

- Installing the library is supported (I might have made configuration mistakes) but not really recommended. I can't actually think of a good use case for doing so.

- It does not make any use of `printf` and friends, so (unless your code uses them) it is safe to do [`std::ios_base::sync_with_stdio(false);`](https://en.cppreference.com/w/cpp/io/ios_base/sync_with_stdio).

## Building and Running okiidoku\_cli

```shell
# cd into the cpp directory of the repo first
mkdir build/Release
pushd build/Release
conan install ../../ --build=missing #--profile=
popd
cmake -S . -B build/Release -DCMAKE_BUILD_TYPE=Release
cd build/Release
cmake --build . --config=Release
./apps/cli/okiidoku_cli
```

You can use `tput rmam` to disable the terminal's line wrapping, and `tput smam` to enable it again afterward.

## Navigating the Source Code

The project is generally structured following [pitchfork layout conventions](https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs).

The project is set up for use with VS Code. After enabling the recommended extensions, you will need to perform [some initial steps](https://code.visualstudio.com/docs/cpp/cmake-linux#_select-a-kit) to tell VS Code what specific build tools you want to use.
