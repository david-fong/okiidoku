# C++ Notes

- https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines

## Snippets

```sh
# do these in a cmake build folder
cmake --build .
nm -nCD src/okiidoku/libokiidoku.so   # view public symbols of the shared library sorted by address
```

## CMake Things

- https://cmake.org/cmake/help/latest/guide/tutorial/index.html
- https://cliutils.gitlab.io/modern-cmake/
- https://github.com/lefticus/cpp_weekly_game_project

## things I got wrong before

things I got wrong before which I couldn't understand based on gcc's error messages.

- For defining global mutable variables (not constants!) shared between cpp files, declare prefixed with `extern` in a hpp files, and then define it in one of the cpp files. Functions always have external linkage.

- `inline` means a name can have multiple _identical_ definitions. For defining _and defining_ global constants in headers with a single memory address, prefix the definition with `inline`. Same for functions in headers.

- Do not use `static` inside a member function to hold a lambda that captures `this`, since `this` is not always at the same address. Seems obvious in retrospect.

## more

- [file open mode shorthands](https://en.cppreference.com/w/cpp/io/basic_filebuf/open)

- [modern iterators](https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp)

- on Template instantiation
  - https://gcc.gnu.org/onlinedocs/gcc/Template-Instantiation.html
  - https://arne-mertz.de/2019/02/extern-template-reduce-compile-times/

- I can specify base-class members like:
  - Derived::Base::member
    https://en.cppreference.com/w/cpp/language/injected-class-name
  - this.Base->member
  - this->member
  - Base<ARGS>::member

- on Shared Library symbol visibility
  - https://gcc.gnu.org/wiki/Visibility
  - https://www.cs.miami.edu/home/burt/learning/Csc521.091/docs/dsohowto.pdf

- Tricky business with globals in static libraries being inlined into multiple DSOs / executables linked together: [cppcon talk](https://www.youtube.com/watch?v=xVT1y0xWgww&ab_channel=CppCon&t=4m25s). Try to avoid this scenario by avoiding globals, but if need be, make sure DSOs are specified first in the link order. See the questions section at t=33m19s for more info.
