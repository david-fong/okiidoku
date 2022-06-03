
# C++ Okiidoku

## Things to Know

- I use the word "order" to describe incremental grid sizes. `O = 3` refers to a grid order of three, where the grid is 9-by-9.
- In the library source code, I use the name "`O1`" to refer to the value of the grid order, "`O2`" to refer to `O1` to the power of two, and "`O4`" to refer to `O1` to the power of four.
- The grid uses the integer value `O2` to represent an empty grid cell, and integers in the range `[0, O2)` to represent non-empty values of a grid.

## Using the Library

- Requirements to build from source:
  - CMake 3.21
  - a C++ compiler [supporting C++20](https://en.cppreference.com/w/cpp/compiler_support). I have tested on GCC 11, Visual Studio 17, and Clang 14. I have no interest in supporting other compilers (ex. apple-clang) or lower versions of the above compilers.
  - a build system (ex. Ninja, Visual Studio)

- The library uses templates for each compiled grid size for optimization purposes. The templates are accessible under the `okiidoku::mono` namespace, and a visitor-pattern interface is exposed under the `okiidoku::visitor` namespace. The visitor pattern is intended to make it more convenient to write code that uses an order selected at runtime.
  - To change the supported grid sizes that get compiled, create a [tweak header](https://vector-of-bool.github.io/2020/10/04/lib-configuration.html#providing-a-tweak-header) for [`./libs/okiidoku/include/okiidoku/config/defaults.hpp`](./libs/okiidoku/include/okiidoku/config/defaults.hpp).

- Make sure to read and follow the api contracts documented in the headers. Exceptions are intentionally not thrown for precondition violations. If you would like to run against a build of libokiidoku with assertions for debugging purposes, use a debug build of libokiidoku.

- Code examples can be found in [the examples folder](./libs/okiidoku/examples/).

- It does not make any use of `printf` and friends, so if nothing in your project uses them either, it is safe to do [`sync_with_stdio(false);`](https://en.cppreference.com/w/cpp/io/ios_base/sync_with_stdio).

## CMake Setup Options

Refer to the [CMake guide](https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html).

- You can build okiidoku and then import from it's build tree.

- If using [`FetchContent`](https://cmake.org/cmake/help/latest/module/FetchContent.html), to fetch from this GitHub repo, declare the [`SOURCE_SUBDIR`] option to point to the `cpp` subdirectory of the repo.

- You can build and then do a system install and then import from the system install, but that's not recommended if you only want to use the library as a dependency. I think a system install would only really make sense if you want to use the cli tool and have it in your system PATH.

## Known Issues

- DLLs are currently not supported for windows builds. I don't know how to properly use the dllimport and dllexport attributes. I do plan to have another crack at it after actually reading the documentation, but it's not one of my priorities.

## Building and Running okiidoku\_cli

If you are familiar enough with CMake, you know what to do and I don't need to give you instructions. If you're not, just use the VS Code CMake-Tools extension to build the project.

You can use `tput rmam` to disable the terminal's line wrapping, and `tput smam` to enable it again afterward.

## Source Code

The project is set up for use with VS Code. After installing and enabling the recommended extensions, you will need to perform [some initial steps](https://code.visualstudio.com/docs/cpp/cmake-linux#_select-a-kit) to tell VS Code what specific build tools ("kit") and build configuration ("variant") you want to use.

Don't open the cpp folder or the repo's root folder in VS Code directly. Open [the repo's workspace file](../okiidoku.code-workspace).

Note: if you installed cmake via snap (like I did on my Ubuntu machine): https://github.com/microsoft/vscode-cmake-tools/issues/838#issuecomment-1035123514

## Acknowledgements

This project wouldn't be possible without the hard work of those who develop

- The C++ language standard and standard library
- GCC, Clang, MSVC, and Emscripten
- CMake and Ninja
- The range-v3 library
- Educational materials for the above technologies
- Computers

Perhaps it's a silly thing to say since it's so obvious, but I can't help but be amazed at what powerful technologies are made available to me at largely no cost. Any inkling of amazement anyone could have at this project pales in comparison to the shoulders of the giants it stands on.