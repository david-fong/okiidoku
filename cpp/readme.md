<!-- SPDX-FileCopyrightText: 2020 David Fong -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
<!-- cspell:dictionaries cpp-refined -->
# C++ Okiidoku

## Things to Know

- I use the word "order" to describe incremental grid sizes. `O = 3` refers to a grid order of three, where the grid is 9-by-9.
- In the library source code, I use the name "`O1`" to refer to the value of the grid order, "`O2`" to refer to `O1` to the power of two, and "`O4`" to refer to `O1` to the power of four.
- The grid uses the integer value `O2` to represent an empty grid cell, and integers in the range `[0, O2)` to represent non-empty values of a grid.

## Using the Library

Recommended tools to install:

```none
reuse g++ clang ninja-build cmake
```
<!-- mold didn't make the cut. see related journal entry -->

- Requirements to build from source:
  - If you are new to C++, you will need an attitude to learn proactively about C++ and CMake.
  - [CMake](https://cmake.org/install/) 4.1 or higher
  - a C++ compiler [supporting C++20/23](https://en.cppreference.com/w/cpp/compiler_support).
    - I test on Clang 20, GCC 14, and Visual Studio 17 2022.
    - I currently have no interest in committing to support other compilers (ex. apple-clang) or lower versions of the above compilers.
    - Older versions _may_ work, but don't expect them to.
  - [a C++ build system](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) (ex. Ninja, Visual Studio)
  - Note: My general policy on tooling version support is to try to support the lowest common denominator of versions that are easily installable on homebrew, apt, and chocolatey, and wait a bit for regressions in new major releases to get fixed.
    - For apt, I try to support the repository for the latest Ubuntu version supported with WSL2.

- The library uses templates for each compiled grid size for optimization purposes. The templates are accessible under the `okiidoku::mono` namespace, and a visitor-pattern interface is exposed under the `okiidoku::visitor` namespace. The visitor pattern is intended to make it more convenient to write code that uses an order selected at runtime.
  - To change the supported grid sizes that get compiled, create a [tweak header](https://vector-of-bool.github.io/2020/10/04/lib-configuration.html#providing-a-tweak-header) for [`./libs/okiidoku/include/okiidoku/config/defaults.hpp`](./libs/okiidoku/include/okiidoku/config/defaults.hpp).

- Make sure to read and follow the API contracts documented in the headers. Exceptions are intentionally not thrown for precondition violations (if you would like to run against a build of libokiidoku with assertions for debugging purposes, use a debug build of libokiidoku). None of the current okiidoku APIs throw exceptions.
  - As a result, for MSVC, any [exception handling policy](https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model#standard-c-exception-handling) should be ok. Note that [CMake currently defaults to `/EHsc`](https://gitlab.kitware.com/cmake/cmake/-/issues/20610) if unspecified.

- Usage examples can be found in [the examples folder](./libs/okiidoku/examples/).

## CMake Setup Options

- Refer to the top-level [CMakeLists.txt](./CMakeLists.txt) file for the list of build options.
- https://cmake.org/cmake/help/latest/guide/user-interaction/index.html

- You can build okiidoku and then import from its build tree.

- If using [`FetchContent`](https://cmake.org/cmake/help/latest/module/FetchContent.html), to fetch from this GitHub repo, declare the [`SOURCE_SUBDIR`] option to point to the `cpp` subdirectory of the repo.

- You can build and then do a system install and then import from the system install.

## Known Issues

- DLLs are currently not supported for Windows builds. I don't know how to properly use the dllimport and dllexport attributes. I do plan to have another crack at it after actually reading the documentation, but it's not one of my priorities.

## Building and Running okiidoku\_cli

You can use CMake from the command line, or just use the VS Code CMake-Tools extension to build the project.

You can use `tput rmam` to disable the terminal's line wrapping, and `tput smam` to enable it again afterward.

## Source Code

The project is set up for use with VS Code. After installing and enabling the recommended extensions, select a configure preset and build preset.
