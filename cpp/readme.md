
# C++ Okiidoku

## Using the Library

- Requirements to build from source:
  - CMake 3.21
  - a C++ compiler [supporting C++20](https://en.cppreference.com/w/cpp/compiler_support). I have tested on GCC 11, Visual Studio 17, and Clang 14. I have no interest in supporting other compilers (ex. apple-clang)
  - a build system (ex. Ninja, Visual Studio)

Refer to the [CMake guide](https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html).

- The library uses templates for each compiled grid size for optimization purposes. The templates are accessible under the `okiidoku::mono` namespace, and a visitor-pattern interface is exposed under the `okiidoku::visitor` namespace. The visitor pattern is intended to make it more convenient to write code that uses an order selected at runtime.
  - To change the supported grid sizes that get compiled, create a [tweak header](https://vector-of-bool.github.io/2020/10/04/lib-configuration.html#providing-a-tweak-header) for [`./libs/okiidoku/include/okiidoku/config/defaults.hpp`](./libs/okiidoku/include/okiidoku/config/defaults.hpp).

- Make sure to read and follow the api contracts documented in the headers. Exceptions are intentionally not thrown for precondition violations. If you would like to run against a build of libokiidoku with assertions for debugging purposes, use a debug build of libokiidoku.

- Code examples can be found in [the examples folder](./libs/okiidoku/examples/).

- It does not make any use of `printf` and friends, so if nothing in your project uses them either, it is safe to do [`sync_with_stdio(false);`](https://en.cppreference.com/w/cpp/io/ios_base/sync_with_stdio).

## Known Issues

- DLLs are currently not supported for windows builds. I don't know how to properly use the dllimport and dllexport attributes. I do plan to have another crack at it after actually reading the documentation, but it's not one of my priorities.

## Building and Running okiidoku\_cli

If you are familiar enough with CMake, you know what to do and I don't need to give you instructions. If you're not, just use the VS Code CMake-Tools extension to build the project.

You can use `tput rmam` to disable the terminal's line wrapping, and `tput smam` to enable it again afterward.

## Source Code

The project is set up for use with VS Code. After installing and enabling the recommended extensions, you will need to perform [some initial steps](https://code.visualstudio.com/docs/cpp/cmake-linux#_select-a-kit) to tell VS Code what specific build tools ("kit") and build configuration ("variant") you want to use.

Don't open the cpp folder or the repo's root folder in VS Code directly. Open [the repo's workspace file](../okiidoku.code-workspace).

Note: if you installed cmake via snap (like I did on my Ubuntu machine): https://github.com/microsoft/vscode-cmake-tools/issues/838#issuecomment-1035123514

