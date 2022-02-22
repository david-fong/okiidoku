# C++ Notes

## things I got wrong before

things I got wrong before which I couldn't understand based on gcc's error messages.

- For defining global variables (not constants!) shared between cpp files, declare prefixed with `extern` in a hpp files, and then define it in one of the cpp files. Functions always have external linkage.

- `inline` means a name can have multiple _identical_ definitions. For defining _and defining_ global constants in headers with a single memory address, prefix the definition with `inline`. Same for functions in headers.

- Do not use `static` inside a member function to hold a lambda that captures `this`, since `this` is not always at the same address. Seems obvious in retrospect.

## more

- https://arne-mertz.de/2019/02/extern-template-reduce-compile-times/