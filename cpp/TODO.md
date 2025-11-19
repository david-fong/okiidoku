<!-- SPDX-FileCopyrightText: 2020 David Fong -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
<!-- cspell:dictionaries cpp-refined, okiidoku-cmake, okiidoku-cpp -->
# Things To Do

## Roadmap

- canonicalization
- puzzle serdes
- archiving
- improve solver and puzzle-maker for order=5
- compare backtracking and stochastic search statistics
- implement an extras module for serdes of different string formats of grids that other people use (translator)
- create a separate github repo for data. add as submodule?
- emscripten build and website
- puzzle creation experiments (see [research-questions.md](../writings/research-questions.md))
- write some example programs using `std::async`.
- language bindings for other languages [see dedicated todo-file](./extras/todo.md)

## Periodic Checkups

- `reuse lint` (check licensing)
- Run static analyzers
- Examine contents of exported symbols (`nm -nCD build/<config>/lib/libokiidoku.so | less`) and make sure nothing is exported that isn't intended to be.
- Ensure dependencies that should be private to the library implementation aren't exposed in any way to the library interface.
- Check if any `NOLINT` comments are no longer needed
- Check if GCC suggest attribute `pure` or `const` anywhere (disabled for default dev flags because GCC14 is noisy for `[[assume(expr)]]`).
- Check if any `OKIIDOKU_CONTRACT2` can be changed to `OKIIDOKU_CONTRACT` (see if Clang warns about potential side effects in `[[assume(expr)]]`).
- Check if any `static_cast`s are no longer needed
- Check which `#ifdef __EMSCRIPTEN__` / `__cpp_lib_...` blocks can be removed as emscripten updates its sysroot libc++.
- https://en.cppreference.com/w/cpp/language/rule_of_three
- Make sure global `constexpr` variables in headers are declared `inline`.
- Write comments for custom CMake commands and targets
- toggle `--warn-uninitialized` CMake flag in settings.json and check warnings.
- places to use `[[gnu::designated_init]]`
- find places `<O>(` is used, and see if deduction guides can help. Ex. `template<typename _Type, size_t _ArrayExtent> span(array<_Type, _ArrayExtent>&) -> span<_Type, _ArrayExtent>;`

## Issues I'm watching:

- https://github.com/emscripten-core/emscripten/discussions/25314 embind value types for C++ integer wrapper classes.
  - related: https://github.com/emscripten-core/emscripten/discussions/18813. try out writing it myself?
- JS `using` statements. [standardization](https://github.com/tc39/proposal-explicit-resource-management?tab=readme-ov-file#status) and [browser support](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Statements/using#browser_compatibility).
- https://github.com/doxygen/doxygen/issues/8159 copydoc with templated functions
- https://gitlab.kitware.com/cmake/cmake/-/issues/26118 cmake non-config presets don't implicitly inherit their config preset condition.
- [Use CMAKE_CROSSCOMPILING_EMULATOR to run cross compiled executables #554](https://github.com/microsoft/vscode-cmake-tools/issues/554)
- [When debugging a target, use debugger.workingDirectory from the CMake file API #4595](https://github.com/microsoft/vscode-cmake-tools/issues/4595)
- https://gitlab.kitware.com/cmake/cmake/-/issues/15179 -Og default for debug build configuration
- https://gitlab.kitware.com/cmake/cmake/-/issues/26092 cmake refer to build preset name in preset for installDir
- `std::ranges::iota()` clang libc++ doesn't have it yet :( (which impacts emscripten build) https://github.com/llvm/llvm-project/issues/105184
- https://gitlab.kitware.com/cmake/cmake/-/issues/20256 CMake mechanism for split debug info. see also https://sourceware.org/gdb/current/onlinedocs/gdb.html/Separate-Debug-Files.html
- https://github.com/emscripten-core/emscripten/issues/25490 emscripten emcc link `make_js_executable`.

## Misc List

- try to consteval default constructors? and copy constructors? parameterless functions.
- try https://clang.llvm.org/docs/AttributeReference.html#regcall if optimizing in a non-portable way. look for a flag to make it default?
- try adding https://clang.llvm.org/docs/AttributeReference.html#noescape to function params where implementation is in separate unit from header.
- try https://clang.llvm.org/docs/AttributeReference.html#nodebug
- try using pcg again
- try deleting array subscript operator for bounded `Int` that would be out of range? test if it actually helps, or if `Int`'s implicit conversion to builtin int subverts it from being meaningful.
- I wonder if [CMake deferred command calls](https://cmake.org/cmake/help/latest/command/cmake_language.html#deferring-calls) could be useful for anything...
- make canon functions pure producers of a canonicalizing transformation. take grid by const reference. name `canon_sym` -> `get_sym_canon_map`.
- see if helpful to add `/// \cond detail` wrapper to `detail` namespaces to suppress doxygen for them

- https://www.pcg-random.org/using-pcg-cpp.html
  https://github.com/imneme/pcg-cpp/tags
  - https://github.com/apache/arrow/pull/35597/files vendor to get rid of the static_arbitrary_seed
- https://squaresrng.wixsite.com/rand

- reproducible builds for msvc: /experimental:deterministic, /d1nodatetime, /Brepro

- MSVC presets: https://gitlab.kitware.com/cmake/cmake/-/issues/21567#note_1141922
  `UseMultiToolTask:true`
  `EnforceProcessCountAcrossBuilds:true`
  `EnableClServerMode:true`
  `BuildPassReferences:true`

- https://developer.chrome.com/blog/faster-wasm-debugging/

- can emscripten just install/package the runtime component? or how can I make it not include headers in the package? https://cmake.org/cmake/help/book/mastering-cmake/chapter/Packaging%20With%20CPack.html#cpack-and-cmake-install-commands
- make repl support custom streams? or should that just be handled by caller of the program? related: support saving session history files that can be later passed as program stdin to repro.

- pgo external project check cache args defined or not to mirror

- figure out if it's possible to config gdb to not show `DOCTEST_CAPTURE_N` things in variables view? or make them show nicer...

- https://youtu.be/zCzD9uSDI8c?t=620

- investigate https://gcc.gnu.org/onlinedocs/libstdc++/manual/parallel_mode_using.html

### waiting

- CMake 4.2
  - Should I define `_UNICODE` or `_SBCS`? See [CMP0204](https://cmake.org/cmake/help/latest/policy/CMP0204.html) and [MFC docs](https://learn.microsoft.com/en-us/cpp/atl-mfc-shared/unicode-and-multibyte-character-set-mbcs-support). Should it be left to user?
- CMake 3.31:
  - presets `$comment`
  - `CMAKE_EXPORT_BUILD_DATABASE`
- GCC 15:
  - `import std` at least in source files? https://www.kitware.com/import-std-in-cmake-3-30/
    - tried on 251024 and ran into issues. presumably because module support is still experimental in my compilers.
- C++23:
  - http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2214r0.html#does-adjacent-mean-2-or-n
  - `std::ranges::iota()` see above note in issues watching
  - `std::views::chunk()`
    - could this be useful for `canon_pos`? I tried messing around with range-v3 on compiler explorer and had trouble with the whole action vs algorithm, container vs view thing. Didn't know what I was doing and could achieve what I wanted.
  - alternative to `std::chunk`, look into `mdspan` (multi-dimensional span). Seems like this is more of what I'm looking for.
  - multidimensional subscript operator
- C++26:
  - https://wg21.link/P2169R4 placeholder variables with no name
  - [postcondition assertions](https://en.cppreference.com/w/cpp/language/function.html#Postcondition_assertions)
    - https://en.cppreference.com/w/cpp/language/contracts.html
    - https://en.cppreference.com/w/cpp/language/contract_assert.html
    - https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2900r14.pdf supported for defaulted special member functions! see section 3.3.3.
    - `-fcontract-evaluation-semantic`
  - `to_chars_result::operator bool`
  - with reflection, I might be able to do what I'm currently doing with macros like `OKIIDOKU_FOREACH_O_EMIT` and `OKIIDOKU_FOREACH_O_DO_EMIT`, and be in a better position to use C++ modules :O.
  - try using `pre` and `post()` on default and copy constructors of `Int<...>` to describe pre and postconditions
  - https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2786r13.html#introduction relocatability
    - https://en.cppreference.com/w/cpp/language/class_property_specifiers.html
  - https://en.cppreference.com/w/cpp/utility/from_chars_result.html `operator bool`
  - [`inplace_vector`](https://en.cppreference.com/w/cpp/container/inplace_vector.html)
- C++29:
  - class invariants?: https://youtu.be/gtFFTjQ4eFU?t=873

### cont.

- https://youtu.be/7QNtiH5wTAs?t=7003 -Wl,--gc-sections does this help? does it mess with my SO interface?
  - what about functions that I want defined only for debugging? see also
    - https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-retain-variable-attribute
    - https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-retain-function-attribute and https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-used-function-attribute
    - https://clang.llvm.org/docs/UsersManual.html#cmdoption-feliminate-unused-debug-types
- https://youtu.be/7QNtiH5wTAs?t=7200 -Wl,--icf=...
- https://youtu.be/7QNtiH5wTAs?t=7322 -Wl,-s (strip-all)

- figure out why nanobind stub generation + asan is having issues. see also https://github.com/google/sanitizers/issues/796#issuecomment-294388904.
  - or it's now addressed? https://github.com/wjakob/nanobind/pull/1000
    - TODO: wait for https://github.com/wjakob/nanobind/pull/1000#issuecomment-2820231142 to get addressed and update dep
  - https://github.com/wjakob/nanobind/discussions/623#discussioncomment-10238357
  - https://drjit.readthedocs.io/en/latest/debug.html#sanitizing-python-sessions

- ./out/install/\<namespace directory\> := build preset name. can't see how to do this right now with there being no macro for buildPresetName in CMake Preset spec. https://gitlab.kitware.com/cmake/cmake/-/issues/26092
- install nanobind stuff

- use CMake configure preset starting point for packaging workflow

check `CMAKE_LANG_COMPILER_FRONTEND_VARIANT` to use msvc flags when compiler is clang-cl?

- write a minimal contributing.md file. examples:
  - https://github.com/libgit2/libgit2/blob/main/docs/contributing.md

- profile for `make_cols_valid` not found in profile data.
  - could this be due to anonymous/unnamed namespaces?
    - If so, how could I fix this?
  - it's being inlined for both the training and optimized builds... do I need to make it not get inlined for the training build?

- ctest for emscripten configuration

usage docs for CMake:
- building an app target (cmake --build . --config=Release --target okiidoku_cli)
- installing an app target (cmake --install . --config=Release --component okiidoku_runtime \[--prefix\])

<!-- - no more separate apply. find will do apply automatically -->
- how to prevent program stack growth due to recursion for singles? consider giving CandPovs an internal data-only stack.
<!-- - should unwinding be non-automatic? no. still no. I see no additional benefit from non-automatic, and still see the downside (opportunity for mistakes in engine usage). -->
<!-- - should UnwindInfo say whether anything was found? yes. please add a bit. -->
<!-- -   why? as a replacement for the FoundQueues functionality of checking if anything was found. -->
<!-- - what should UnwindInfo now be called? LogicStat? FindStat? FindDigest? FindSummary? FindErrc? I like FindStat the most. -->
- consider making subset finders take parameter of _single_ subset size to try finding for instead of a ceiling-like parameter.

consider making cand masks have two lanes: one storing full O2BitArr, one storing just a byte. the byte lane is intended for use with subset finding and needs an additional array of o2x\_t where indices (relative to the index of the beginning of the subset) correspond to bits of the byte mask, and the value is the value of the candidate (candidate tags). Then to remove a given candidate, can use simd to scan the candidate tags and get a byte where bits are set if the corresponding tag value matched the cand to remove, and then remove set bits of that byte from the byte storing subset candidates. <https://stackoverflow.com/questions/54897297/check-all-bytes-of-a-m128i-for-a-match-of-a-single-byte-using-sse-avx-avx2>

- give this a try [](https://crascit.com/2022/06/24/build-performance-insights/)

- Come up with a precondition-checking strategy for language bindings. Options:
  - Leave it as a user responsibility to know and follow contracts.
  - Throw exception on the other language side.

- think about [this](https://wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#Pseudorandom_generators)

- Collect some statistics on how much overlap there usually is between the size-4 UA sets found. A very narrow, simple way to do this is just count how many unique cells are covered, and compare to the number of non-unique cells covered.
  - links to UA set discussions:
    - http://forum.enjoysudoku.com/post318692.html?hilit=unavoidable#p318692
    - http://forum.enjoysudoku.com/unbiased-grid-generation-t5128-15.html#p39512
    - http://forum.enjoysudoku.com/structures-of-the-solution-grid-t4235-75.html

- try focusing guesses on the cell ruling out a known solution
- see the TODO for `get_guess_grouping`.
- try to use [unavoidable set](https://www.sudopedia.org/wiki/Unavoidable_Set) / deadly pattern detection to optimize proper-puzzle-making.
  - [interesting](http://forum.enjoysudoku.com/max-number-of-clues-t1448.html#p21531).
- possible "deduction technique": given a cell that has multiple candidates, try guessing each candidate (to create "derivatives") and see how much more can be deduced for each derivative without further guessing after that. Any candidates that are ruled out for all the derivatives can be ruled out from the base.
- consider a finder-heuristic that only starts using more powerful finders when the guess stack gets to a certain depth- at which point it unwinds to one-above the last frame that used more powerful finders, and uses more powerful finders.
  - Hm. But the guess stack depth doesn't necessarily correspond to when it may make the most sense to again use more expensive deduction techniques, since I don't know if there's any saying how much further deduction a guess may enable.
  - will have to create some modified version of unwinding which doesn't rule out the guess.
  - or more fine grained: each guess stack frame has a "logic depth/effort" field that describes the finders that were used before the guess was made. Ex. used up to finding subsets of size 2 before the guess was made.
- play around with giving puzzle maker a threshold to stop trying to solve a candidate puzzle after N guesses, or give up (or mark as try-again-later) seeing if a given is safe to remove after N guesses.

- Interesting: [](https://cmake.org/cmake/help/latest/variable/CMAKE_VERIFY_INTERFACE_HEADER_SETS.html)

- cppcoreguidelines C.2: use class if there is an invariant, and struct otherwise.
  - hm. I mean- for a lot of my really simple structs there are invariants on integer bounds. Seems like judgement is required to choose where to draw the line with this guideline.

- CI/CD
  - [CPM notes](https://github.com/cpm-cmake/CPM.cmake/wiki/Caching-with-CPM.cmake-and-ccache-on-GitHub-Actions#caching-with-github-actions)
  - [catch2 notes](https://github.com/catchorg/Catch2/blob/devel/docs/ci-and-misc.md)

- find out how to use the [cppcoreguidelines checker](https://docs.microsoft.com/en-us/cpp/code-quality/using-the-cpp-core-guidelines-checkers?view=msvc-170)
  - I want to use it in a way that is driven by cmake

- see if grid qualities (like being a solution, being a proper puzzle, being a minimal puzzle), can be encoded through the type system and make it so that always-safe conversions (such as ) are easy, but "unsafe" (not always true) have to either go through an `unsafe_cast_X_grid_to_Y_grid` function, or go through a `checked_cast_X_grid_to_Y_grid`, which may have a non-trivial performance penalty.
  - This would allow making many of the current contracts part of the type system; turning-runtime-error-into-compiler-errors-TM. I would no longer need to write such contract and post-condition comments.
  - Definitely useful types: SolutionGrid, ProperPuzzleGrid, MaybeInvalidPuzzleGrid.
  - Maybe useful types: MinimalPuzzleGrid.
  - Things that might get ugly: the way they are, `grid_is_filled` and `grid_is_valid` would still require the caller to do some kind of unsafe-moving-cast to a new variable of a stronger type.

- try DLL's again? read the docs actually.
  - [general rules and limitations](https://docs.microsoft.com/en-us/cpp/cpp/general-rules-and-limitations)

- try writing custom GDB python pretty-printer for `O2BitArr`?
  - [how to write a gdb pretty-printer](https://sourceware.org/gdb/onlinedocs/gdb/Writing-a-Pretty_002dPrinter.html#Writing-a-Pretty_002dPrinter)
  - also look into [visual studio `.natvis` files](https://docs.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects)
  - interesting: https://github.com/asarium/gdb-natvis adapter for GDB that searches for natvis files for types

- move the emoji definitions out of the program binary and externalize as a configurable data read in at runtime?

- Go back and try the old canonicalization by rel row probability, but break ties by doing some brute force: try each tied permutation and valuate it according to some reduction of how it pushes rarer rel counts to the top left. Just be careful to shift to get rid of the main diagonal seam.
  - If there are multiple puddles of ties, the resolution of a puddle shouldn't depend on the resolution of any other puddle- only on the non-tied rows/columns. A consequence of this is that this resolution algorithm will not work if there are no non-tied rows/columns.

1. Consider: The current relabelling canonicalization method may have a big weakness: I think ties can be easily crafted: consider the "Most Canonical" solution grid- it would be all ties. How can this be addressed? (Or perhaps the "Most Canonical" grid is the only weakness?)
    - Break ties by designing a way to give symbols that frequently cohabit atoms label-values that are closer together in value:
    - The cohabitation table can be tiled; a coordinate's vertical or horizontal distance (these are the same, since relabelling moves both the col and row together) from the closest tiling of the main diagonal represents the distance of the labels from each other.
    - For each label, make an array where each index represents another label, and the value is an object containing the left and right distances between them, and the cohabitation count.
    - Hm. So far this seems to suggest a hill-climbing / brute-force-type solution...

- Decide what interfaces to support:
  - Probably best to start with just readline and a CLI
    - For CLI util libraries, look into using
      - repl
        - https://github.com/daniele77/cli
      - command
        - https://github.com/CLIUtils/CLI11
      - FTXUI
  - Can look into ncurses in the future? Or look into options for TUI libraries?
  - A web interface would be really nice.
- CLI
  - implement `-h` and `--help` CLI argument.
  - give a red message when trying to continue and nothing is left to be found.
