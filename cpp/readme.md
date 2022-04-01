
# C++ Ookiidoku

## Using the Dynamic Library

Building ookiidoku requires CMake (with [the minimum specified version](./CMakeLists.txt)), a C++ compiler [supporting C++20](https://en.cppreference.com/w/cpp/compiler_support), a build system like Make or Ninja, and Conan.

Refer to the [CMake guide](https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html). This may also help: [tldp.org on shared libraries](https://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html)

- Make sure to read and follow the api contracts documented in the headers. Exceptions are intentionally not thrown for precondition violations. If you would like to run against a build of libookiidoku with assertions for debugging purposes, create and use a build of libookiidoku with `-DCMAKE_BUILD_TYPE=Debug`.

- To change the supported grid sizes that get compiled, create a [tweak header](https://vector-of-bool.github.io/2020/10/04/lib-configuration.html#providing-a-tweak-header) for [`include/ookiidoku/ookiidoku_config.hpp`](include/ookiidoku/ookiidoku_config.hpp).

- It does not make any use of `printf` and friends, so it is safe to do [`std::ios_base::sync_with_stdio(false);`](https://en.cppreference.com/w/cpp/io/ios_base/sync_with_stdio).

## Building and Running ookiidoku\_cli

```shell
# cd into the cpp directory of the repo first
mkdir build/Release
pushd build/Release
conan install ../../ --build=missing
popd
cmake -S . -B build/Release -DCMAKE_BUILD_TYPE=Release
cd build/Release
cmake --build . --config=Release
./src/ookiidoku_cli/ookiidoku_cli
```

## Navigating the Source Code

The project is generally structured following [pitchfork layout conventions](https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs).

The project is set up for use with VS Code. After enabling the recommended extensions, you will need to perform [some initial steps](https://code.visualstudio.com/docs/cpp/cmake-linux#_select-a-kit) to tell VS Code what specific build tools you want to use.
