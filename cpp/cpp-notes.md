<!-- SPDX-FileCopyrightText: 2020 David Fong -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
<!-- cspell:dictionaries cpp-refined -->
# C++ Notes

## Coding Guides

- [isocpp core guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [pitchfork layout](https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs)
- [google cppguide](https://google.github.io/styleguide/cppguide.html)
- [clang tidy checks list](https://clang.llvm.org/extra/clang-tidy/checks/list.html)

## Project Style Guide

- Try to follow pitchfork layout conventions
- Use Stroustrup naming conventions. In general, use lower-snake with `_t` suffix for type aliases.
- Use `struct` for things where the primary purpose is to _store_ things. Use `class` for where the primary purpose is to _do_ things.
- If an optional CMake target requires fetching large dependencies, guard the `add_subdirectory` with a cmake option; otherwise, mark with EXCLUDE_FROM_ALL.

## Snippets

```sh
# list all words in source code starting with "OKIIDOKU_"
# -o: only-matching; -S smart-case; -I: no-filename ; -N: no-line-number
rg -oSIN --no-filename '(\w+)?okiidoku_\w+' | sort | uniq -c

# (cd into build tree)
# install to a test directory
cmake --install . --config=Release --prefix=install_test

# (cd into build tree)
# view public(-D) de-mangled(-C) symbols of the shared library sorted by address(-n)
nm -nCD build/Release/lib/libokiidoku.so

# (cd into build tree)
# de-mangled(-C) assembler contents of executable sections(-d), intermixing source code with disassembly(-S). --visualize-jumps=extended-color
objdump -SCd build/Release/lib/libokiidoku.so | less

# (cd to the source tree)
# view the grid translation unit with macros fully expanded
g++ -E -I../libs/okiidoku/include/ -Ilibs/okiidoku/Release/include/ ../libs/okiidoku/src/okiidoku/grid.cpp | less

# https://perf.wiki.kernel.org/index.php/Tutorial
perf stat -e branch-misses:u ./build/Release/bin/okiidoku_test

# compare sizes of pgo vs release builds
size ./build/PgoUse/lib/libokiidoku.so ./build/Release/lib/libokiidoku.so

# (cd into build tree)
gdb --cd=build/Debug bin/okiidoku_test.o2_bit_arr

# (cd into build tree)
# list all tests
ctest --show-only

# (cd into build tree)
du -L --si -a out/install/dev.gcc/{lib,bin}
du -L --si -a out/install/dev.gcc/{lib,bin} | sort -n

# (cd into build tree)
# https://crascit.com/2022/06/24/build-performance-insights/
../../../tools/ninjatracing.py .ninja_log > trace.json

# read ELF file of libokiidoku
readelf -a --wide --demangle lib/libokiidoku.so | less

# TODO no longer needed once https://github.com/wjakob/nanobind/pull/1000 further improvements are made
LD_PRELOAD="$(g++ -print-file-name=libasan.so):$(g++ -print-file-name=libubsan.so)" cmake --build --preset=dev.gcc.debug
LD_PRELOAD="$(clang++ -print-file-name=libasan.so):$(clang++ -print-file-name=libubsan.so)" cmake --build --preset=dev.clang.debug

# print something like a minified version of a c++ file
# unfortunately -dI doesn't seem to be effective with this flag combo
g++ -I libs/okiidoku/include/ -E libs/okiidoku/include/okiidoku/ints.hpp -P -dI -fmax-include-depth=0 | less
```

## CMake Things

- [official tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)
- [cmake for dynamic library developers - craig scott](https://www.youtube.com/watch?v=m0DwB4OvDXk)
- [alex reinking tutorial part 2](https://alexreinking.com/blog/how-to-use-cmake-without-the-agonizing-pain-part-2.html)
- [](https://crascit.com/2016/01/31/enhanced-source-file-handling-with-target_sources/)
- [preparing a project for CPM](https://github.com/cpm-cmake/CPM.cmake/wiki/Preparing-projects-for-CPM.cmake)
- https://cliutils.gitlab.io/modern-cmake/
- [Jason Turner example/template](https://github.com/lefticus/cpp_weekly_game_project)
- [](https://cmake.org/cmake/help/latest/command/install.html#install-runtime-dependency-set)

- <https://cmake.org/cmake/help/latest/variable/PROJECT-NAME_SOURCE_DIR.html>

- presets:
  - [cmake docs](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
  - [vscode extension docs](https://github.com/microsoft/vscode-cmake-tools/blob/main/docs/cmake-presets.md)

## Reuse SPDX notes

- place the comment tags in the earliest possible line that a comment can go.
- which licence to use?
  - source code: AGPL-3.0-or-later
  - tests and example code: AGPL-3.0-or-later
  - build system config: GPL-3.0-or-later
  - debugger helper config: GPL-3.0-or-later
  - documentation: CC0-1.0
  - other writings: CC0-1.0
  - small "helper tool" configs (gitignore, linting, formatting, IDE): CC0-1.0

<https://reuse.software/dev/>
<https://www.gnu.org/licenses/gpl-faq.html>
<https://www.gnu.org/licenses/gpl-howto.html>
<https://www.gnu.org/licenses/license-list.en.html>

## things I got wrong before

things I got wrong before which I couldn't understand based on gcc's error messages.

- For defining global mutable variables (not constants!) shared between cpp files, declare prefixed with `extern` in a hpp files, and then define it in one of the cpp files. Functions always have external linkage. Note: but is there a good design argument for having that global mutable variable?

- `inline` means a name can have multiple _identical_ definitions. For declaring _and defining_ global constants in headers with a single memory address, prefix the definition with `inline`. Same for functions in headers. For functions, if the function makes sense to make constexpr, use constexpr instead.

- Do not use `static` inside a member function to hold a lambda that captures `this`, since `this` is not always at the same address. Seems obvious in retrospect.

- Emscripten does not support the `-march=` flag. https://github.com/emscripten-core/emscripten/issues/20244#issuecomment-1722510550

## more

- [equality and ordering operators](https://brevzin.github.io/c++/2019/07/28/comparisons-cpp20/)

- on Shared Library symbol visibility
  - https://gcc.gnu.org/wiki/Visibility
  - https://www.cs.miami.edu/home/burt/learning/Csc521.091/docs/dsohowto.pdf

- on Template instantiation
  - https://gcc.gnu.org/onlinedocs/gcc/Template-Instantiation.html
  - https://arne-mertz.de/2019/02/extern-template-reduce-compile-times/

- [nodiscard in the standard library](https://stackoverflow.com/a/63204963/11107541)
  - this can inform where I should use nodiscard in my library interfaces.

- [constness and logical-vs-physical state](https://isocpp.org/wiki/faq/const-correctness#logical-vs-physical-state)
- [misuse of the gnu const attribute on functions](https://github.com/OSGeo/PROJ/issues/1224)

- [file open mode shorthands](https://en.cppreference.com/w/cpp/io/basic_filebuf/open)

- [move simply - herb sutter](https://herbsutter.com/2020/02/17/move-simply/)
- [forwarding](https://medium.com/factset/modern-c-in-depth-perfect-forwarding-570f242261f8)

- [modern iterators](https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp)

- [stream operator for class should go in same namespace as that class](https://stackoverflow.com/a/2479542/11107541)

- [using/inheriting base-class constructor](https://en.cppreference.com/w/cpp/language/using_declaration#Inheriting_constructors)

- [non-deduced contexts and template-argument deduction](https://en.cppreference.com/w/cpp/language/template_argument_deduction#Non-deduced_contexts)

- [cpp core guidelines - do not over-parametrize members](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#t61-do-not-over-parameterize-members-scary)

- [MSVC build time profiling](https://learn.microsoft.com/en-us/cpp/build-insights/?view=msvc-170)
- https://slides.com/onqtam/faster_builds#/48/0/10

- I can specify base-class members like:
  - Derived::Base::member
    https://en.cppreference.com/w/cpp/language/injected-class-name
  - this.Base->member
  - this->member
  - Base<ARGS>::member

- Tricky business with globals in static libraries being inlined into multiple DSOs / executables linked together: [cppcon talk](https://www.youtube.com/watch?v=xVT1y0xWgww&ab_channel=CppCon&t=4m25s). Try to avoid this scenario by avoiding globals, but if need be, make sure DSOs are specified first in the link order. See the questions section at t=33m19s for more info.

- [what's in a class? - herb sutter](http://www.gotw.ca/publications/mill02.htm)
- [pitfalls of ADL](https://stackoverflow.com/questions/2958648/what-are-the-pitfalls-of-adl)

- [How to write well-behaved value-wrappers](https://www.youtube.com/watch?v=J4A2B9eexiw&ab_channel=CppCon).

- [](https://gcc.gnu.org/onlinedocs/gcc/Compatibility.html)

- [non-exported dependencies of exported static libraries](https://gitlab.kitware.com/cmake/cmake/-/issues/17357)

- [moving to c++ modules](https://devblogs.microsoft.com/cppblog/moving-a-project-to-cpp-named-modules/)
  - [](https://devblogs.microsoft.com/cppblog/cpp20-modules-in-cmake-with-vs/)

- compiler "assume" hints (asserts are checked at runtime. assumptions are not checked and used for optimizations (to do the opposite: "remove checks")).
  - [GSL](https://conan.io/center/ms-gsl) has a macro ["`GSL_ASSUME`"](https://github.com/microsoft/GSL/blob/main/include/gsl/assert) to do this on MSVC, Clang, and GCC.
  - [standardization proposal](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p1774r4.pdf)
  - [interesting clang thing](https://github.com/microsoft/GSL/pull/608)

- [floats in `(0.0,1.0]` can be slower to process](https://en.wikipedia.org/wiki/Subnormal_number#Performance_issues) (apparently?)

## misc VS Code regexes

filter source code: `*.hpp,*.cpp`

```regex
static_cast<(int_ts|o[1-5])
const [^{]*&
[>=]= \S+::O2
```
