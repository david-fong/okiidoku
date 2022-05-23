# C++ Notes

## Coding Guides

- [isocpp core guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [pitchfork layout](https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs)
- [google cppguide](https://google.github.io/styleguide/cppguide.html)

## Project Style Guide

- Try to follow pitchfork layout conventions
- Use Stroustrup naming conventions. In general, use lower-snake with `_t` suffix for type aliases.
- Use `struct` for things where the primary purpose is to _store_ things. Use `class` for where the primary purpose is to _do_ things.

## Snippets

```sh
# (cd to the cmake build folder of interest)
# view public (-D) de-mangled (-C) symbols of the shared library sorted by address (-n)
nm -nCD libs/okiidoku/libokiidoku.so

# (cd to the cpp folder)
# view the grid translation unit with macros fully expanded
g++ -E -Icpp/libs/okiidoku/include/ -Icpp/build/Release/libs/okiidoku/ cpp/libs/okiidoku/src/okiidoku/grid.cpp | less
```

## CMake Things

- [official tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)
- https://cliutils.gitlab.io/modern-cmake/
- [Jason Turner example/template](https://github.com/lefticus/cpp_weekly_game_project)
- [cmake for dynamic library developers](https://www.youtube.com/watch?v=m0DwB4OvDXk)

## Conan Things

- [Using conan with CMake without making the CMake config depend on conan](https://jfreeman.dev/blog/2019/06/27/trying-conan-with-modern-cmake:-packaging/)

## things I got wrong before

things I got wrong before which I couldn't understand based on gcc's error messages.

- For defining global mutable variables (not constants!) shared between cpp files, declare prefixed with `extern` in a hpp files, and then define it in one of the cpp files. Functions always have external linkage. Note: but is there a good design argument for having that global mutable variable?

- `inline` means a name can have multiple _identical_ definitions. For defining _and defining_ global constants in headers with a single memory address, prefix the definition with `inline`. Same for functions in headers. Update: for constants, if the type supports constexpr construction, use constexpr instead. For functions, if the function makes sense to make constexpr, use constexpr instead.

- Do not use `static` inside a member function to hold a lambda that captures `this`, since `this` is not always at the same address. Seems obvious in retrospect.

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

- [modern iterators](https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp)

- [stream operator for class should go in same namespace as that class](https://stackoverflow.com/a/2479542/11107541)

- [using/inheriting base-class constructor](https://en.cppreference.com/w/cpp/language/using_declaration#Inheriting_constructors)

- [non-deduced contexts and template-argument deduction](https://en.cppreference.com/w/cpp/language/template_argument_deduction#Non-deduced_contexts)

- [cpp core guidelines - do not over-parametrize members](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#t61-do-not-over-parameterize-members-scary)

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