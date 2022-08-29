

- how are other people doing it?
  - llvm/clang:
    - [user docs](https://llvm.org/docs/HowToBuildWithPGO.html)
  - firefox:
    - [user docs](https://firefox-source-docs.mozilla.org/build/buildsystem/pgo.html)
    - [search repo for flag](https://searchfox.org/mozilla-central/search?q=-fprofile-generate&path=&case=false&regexp=false)
    - [the file found](https://searchfox.org/mozilla-central/source/build/moz.configure/lto-pgo.configure)
    - [file with flags for prefix-map](https://searchfox.org/mozilla-central/source/build/moz.configure/flags.configure)

- reproducible builds / deterministic compilation
  - <https://reproducible-builds.org/>
    - <https://reproducible-builds.org/who/projects/>
  - [llvm blog](https://blog.llvm.org/2019/11/deterministic-builds-with-clang-and-lld.html)
  - [blog post by conan](https://blog.conan.io/2019/09/02/Deterministic-builds-with-C-C++.html#randomness-created-by-the-compiler)
  - [someone using visual studio + bazel](https://nikhilism.com/post/2020/windows-deterministic-builds/)