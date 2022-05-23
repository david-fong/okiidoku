
Some people advocate for "cmake-non-intrusive" package-manager usage. Ie. keeping the cmake configuration agnostic with respect to the package manager.

- I want to use range-v3
- I look at my package manager options.
- No clear "best" option. Some people who do comparisons seem to like conan.
- I feel uneasy about vcpkg since it was first created just for visual studio or something and then later made to support cross-platform. What if it isn't very environmentally friendly/clean to linux and mac?
- I decide to try conan.
- It feels weird that I have to install it through pip, but ok.
- It works for my ubuntu, windows, and mac, with GCC, Clang, and MSVC.
- Yay!
- I try using it with emscripten.
- It doesn't work.
- Emscripten's bundled libc++ was the one for LLVM 13, which doesn't have some three-way-comparison things that I use.
- Check GitHub. Pleasant surprise- the emscripten team [_just_ bumped the bundled libc++ to LLVM 14's one](https://github.com/emscripten-core/emscripten/pull/17000).
- Yay!
- Oh wait- there's more problems.
- Turns out range-v3 0.11.0 seems to have [some compat issues with newer libc++ versions](https://github.com/ericniebler/range-v3/issues/1633).
- I could switch to vcpkg, or I could try ditching package managers altogether.
- Maybe use git submodules, or CMake's ExternalProject.
- hmm... I want something where I can just run cmake configure and it will do all the fetching automatically, so git submodules is not super ideal (need to explicitly clone into submodules and maybe even specifically select which to recursively clone, and then to a build and hope the library's cmake config allows importing from the build-tree).
- Oh turns out there's FetchContent, which wraps ExternalProject and runs at configure-time instead of build-time.
- Cool. Let's try it.
- So now my build is getting tons of warnings about range-v3 using c-style casts (a warning I'd like to get if _my own_ code does that).
- Whyyyyyyyyyyyyyyyy.
- Hmm... I'll change my compile options stuff to use an interface library and link that interface library to all my targets.
- Still getting the warnings
  - (seems obvious why in retrospect).
- Whyyyyyyy. This wasn't happening before.
- Ok. It's because the range-v3 headers are getting added to the include path with `-I` instead of `-isystem` like they would be if it was added as an IMPORTED target ([a handy CMake feature](https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#include-directories-and-usage-requirements) that I was benefiting from when I was using Conan).
  - here's [another link](https://cmake.org/cmake/help/latest/prop_tgt/NO_SYSTEM_FROM_IMPORTED.html)
- FetchContent uses add_subdirectory instead of using the IMPORTED targets mechanisms like package managers would.
- The IMPORTED target property is read-only (can only be set by the call to add_library).
- Oh but there's [a slightly ugly hack](https://stackoverflow.com/questions/64064157/is-there-a-way-to-get-isystem-for-fetchcontent-targets) I can use: just manually move the interface include directories to be interface system include directories.
- It's worth noting that if I was using a library that wasn't header-only and didn't do any in-header definitions, I would be much less likely to run into this compiler warning and `-isystem` thing.
- The hack isn't pretty and it's not very scalable. I mean- I only have one dependency so I don't need to scale, but how do people navigate all these problems? (the answer is probably package managers).
- So should I try vcpkg?
- I feel so utterly done with running into walls like this. Did I bring this upon myself? I guess yeah since I'm being really picky.

- There are a couple of things range-v3 could do that would help me (of course- I'm not entitled to its help for my specific use case. I know that. I'm just doing some thinking):
  - Use really strict compile compiler warning options on itself such as disallowing c-style casts.
    - Then I wouldn't care that its headers get added with `-I` instead of `-isystem` when added with FetchContent.
    - Oh! But it turns out that range-v3 _is_ extremely strict on itself. It just _specifically_ opted out of the old-style-cast warning. See [this](https://github.com/ericniebler/range-v3/blob/master/cmake/ranges_diagnostics.cmake). Oh.
  - [Publish a new release](https://github.com/ericniebler/range-v3/issues/1694) (0.11.0 was in 2020). Conan only publishes official releases as tagged by package authors, unlike vcpkg, which seems to package newer versions "without consent" of the package authors. There was even [an issue](https://github.com/conan-io/conan-center-index/issues/8386) and PR for conan center to do this, but the PR died without getting reviewed.
    - Then I might not have run into the mouth of this little rabbit hole at all.

- There's also the fact that FetchContent isn't super easy to share between multiple projects. You can set SOURCE_DIR to be a common directory, such as an `external` directory, but somewhat disappointingly, if you remove the build directory and reconfigure, CMake will try to clone again even though the clone is still there in the `external` directory untouched/unchanged.
  - [link to CMake discourse thread](https://discourse.cmake.org/t/share-fetchcontent-between-projects/4537/2)