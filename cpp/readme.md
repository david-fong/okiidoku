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
g++ clang ninja-build cmake
ccache lld reuse clang-tidy iwyu # optional devtooling
```
<!-- mold didn't make the cut. see related journal entry -->

- Requirements to build from source:
  - [CMake](https://cmake.org/install/) 4.1 or higher
  - a recent C++ compiler [supporting most of C++23](https://en.cppreference.com/w/cpp/compiler_support).
    - I test on Clang 20, GCC 15, and (much less frequently,) Visual Studio 17 2022. I can't guarantee things will work for you.
  - [a C++ build system](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) (ex. Ninja, Visual Studio, Make)
  - Note: My general policy on tooling version support is to try to support the lowest common denominator of versions that are easily installable on homebrew, apt, and chocolatey, and wait a bit for regressions in new major releases to get fixed.
    - For apt, I try to support the repository for the latest Ubuntu version supported with WSL2.

- The library uses templates for each compiled grid size for optimization purposes. The templates are accessible under the `okiidoku::mono` namespace, and a visitor-pattern interface is exposed under the `okiidoku::visitor` namespace. The visitor pattern is intended to make it more convenient to write code that uses an order selected at runtime.
  - To change the supported grid sizes that get compiled, create a [tweak header](https://vector-of-bool.github.io/2020/10/04/lib-configuration.html#providing-a-tweak-header) for [`./libs/okiidoku/include/okiidoku/config/defaults.hpp`](./libs/okiidoku/include/okiidoku/config/defaults.hpp).

- Follow the documented API contracts. Exceptions are intentionally not thrown for precondition violations (if you would like to run against a build of libokiidoku with assertions for debugging purposes, use a debug build of libokiidoku). None of the current okiidoku APIs throw exceptions directly.
  - As a result, for MSVC, any [exception handling policy](https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model#standard-c-exception-handling) should be ok. Note that [CMake currently defaults to `/EHsc`](https://gitlab.kitware.com/cmake/cmake/-/issues/20610) if unspecified.

- Usage examples can be found in [the examples folder](./libs/okiidoku/examples/).

I make no particular commitment to guarantee any degree of ABI stability, or support for a particular toolchain. But if you know how to improve usability of this library with minimal disruption, you're welcome to open a pull request making a case for suggested improvements.

## CMake Setup Options

- Refer to the top-level [CMakeLists.txt](./CMakeLists.txt) file for the list of build options, and [CMakePresets.json](./CMakePresets.json) file for presets you can use (or build off of in a CMakeUserPresets.json).
- See [CMake's User Interaction Guide](https://cmake.org/cmake/help/latest/guide/user-interaction/index.html) for how to invoke CMake.

- If using [`FetchContent`](https://cmake.org/cmake/help/latest/module/FetchContent.html), to fetch from this GitHub repo, declare the [`SOURCE_SUBDIR`] option to point to the `cpp` subdirectory of the repo.

- You can build and then do a system install and then `find_package` or `import` the system install.

- You can build okiidoku and then import from its build tree.

## Known Issues

- DLLs are currently not supported for Windows builds. I don't know how to properly use the dllimport and dllexport attributes. I do plan to have another crack at it after actually reading the documentation, but it's not one of my priorities.

## Building and Running okiidoku\_cli

You can use CMake from the command line, or just use the VS Code CMake-Tools extension to build the project.

You can use `tput rmam` to disable the terminal's line wrapping, and `tput smam` to enable it again afterward.

## Source Code

The project is set up for use with VS Code. After installing and enabling the recommended extensions, [select a configure preset](command:cmake.selectConfigurePreset) and [build preset](command:cmake.selectBuildPreset), and then [build](command:cmake.build).
