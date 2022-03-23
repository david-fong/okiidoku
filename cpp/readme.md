
# C++ Solvent

## Using the Dynamic Library

Building solvent requires CMake (with [the minimum specified version](./CMakeLists.txt)), a C++ compiler [supporting C++20](https://en.cppreference.com/w/cpp/compiler_support), and a build system like Make or Ninja.

As for actually linking to the dynamic library, ¯\\\_(ツ)\_/¯. I don't really know how this stuff works. See [solvent\_cli's CMakeLists.txt](./src/solvent_cli/CMakeLists.txt) for inspiration. Maybe this will help: [tldp.org on shared libraries](https://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html)

- Make sure to read and follow the api contracts documented in the headers. Exceptions are intentionally not thrown for precondition violations. If you would like to run against a build of libsolvent with assertions, create and use a build of libsolvent with `-DCMAKE_BUILD_TYPE=Debug`.

- To change the supported grid sizes that get compiled, create a [tweak header](https://vector-of-bool.github.io/2020/10/04/lib-configuration.html#providing-a-tweak-header) for [`include/solvent/solvent_config.hpp`](include/solvent/solvent_config.hpp).

- It does not make any use of `printf` and friends, so it is safe to do [`std::ios_base::sync_with_stdio(false);`](https://en.cppreference.com/w/cpp/io/ios_base/sync_with_stdio).

## Building and Running solvent\_cli

```shell
# cd into the cpp directory of the repo first
cmake -S . -B build/Release -DCMAKE_BUILD_TYPE=Release
cd build/Release
cmake --build . --config=Release
./src/solvent_cli/solvent_cli
```

## Navigating the Source Code

The project is generally structured following [pitchfork layout conventions](https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs).

The project is set up for use with VS Code. After enabling the recommended extensions, you will need to perform [some initial steps](https://code.visualstudio.com/docs/cpp/cmake-linux#_select-a-kit) to tell VS Code what specific build tools you want to use.
