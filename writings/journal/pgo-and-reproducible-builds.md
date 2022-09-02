

- how are other people doing it?
  - llvm/clang:
    - [user docs](https://llvm.org/docs/HowToBuildWithPGO.html)
  - firefox:
    - [user docs](https://firefox-source-docs.mozilla.org/build/buildsystem/pgo.html)
    - [search repo for flag](https://searchfox.org/mozilla-central/search?q=-fprofile-generate&path=&case=false&regexp=false)
    - [the file found](https://searchfox.org/mozilla-central/source/build/moz.configure/lto-pgo.configure)
    - [file with flags for prefix-map](https://searchfox.org/mozilla-central/source/build/moz.configure/flags.configure)
  - chromium:
    - [repo](https://chromium.googlesource.com/chromium/src/+/refs/heads/main/build/config/compiler/pgo/)

- reproducible builds / deterministic compilation
  - <https://reproducible-builds.org/>
    - <https://reproducible-builds.org/who/projects/>
  - [llvm blog](https://blog.llvm.org/2019/11/deterministic-builds-with-clang-and-lld.html)
  - [blog post by conan](https://blog.conan.io/2019/09/02/Deterministic-builds-with-C-C++.html#randomness-created-by-the-compiler)
  - [someone using visual studio + bazel](https://nikhilism.com/post/2020/windows-deterministic-builds/)

- want pgo
  - support multiple trainers
  - single user config that selects one of the trainers
    - selecting a different one should not strictly require trainee to trainer to rebuild, or training to re-run unless anything is stale.
  - trainee and all trainers can be part of a single config
  - training-data is per-trainer

---

I tried to ask this as a question on stackoverflow.com. I was quite proud of it and thought it was well written. It got closed for being too broad. I feel discouraged, but I understand. You probably don't care, but I want to remember it. So here it is:

I have a CMake project whose main target is a single shared library. I want to set up PGO using CMake.

While I wait for the very busy CMake team to have time to think about the feasibility and approach of [adding a CMake module allowing easy use of PGO](https://gitlab.kitware.com/cmake/cmake/-/issues/19273), I'm trying to figure it out for myself.

Note: This is my first question. If you downvote it, I'd really appreciate a comment to explain why so I can learn how to be a better community member on this site. Thanks for your time.

### Rough table of contents:

- Existing examples of PGO in large projects that I'm aware of
- Full question description
- documentation I have been reading
- Why I (think you might want to, but) hope you don't flag this question as being too broad
- Existing related questions and why I don't think I'm a duplicate
- My current approach and progress (I'd say I'm about 75% there)
- Links to my full CMake config code

##### Here are some big projects that I know can build with PGO, but _not wholly via CMake_:

- FireFox:
  - [user docs](https://firefox-source-docs.mozilla.org/build/buildsystem/pgo.html)
  - [a (python(?)) config file that looks related](https://searchfox.org/mozilla-central/source/build/moz.configure/lto-pgo.configure)
- Chromium:
  - [PGO config things](https://chromium.googlesource.com/chromium/src/+/refs/heads/main/build/config/compiler/pgo/)
  - [it uses `gn` as its meta-buildsystem](https://chromium.googlesource.com/chromium/src/tools/gn/+/48062805e19b4697c5fbd926dc649c78b6aaa138/README.md)

##### LLVM uses CMake for its PGO things

I have done a skim over some of the CMake config files to find relevant keywords. I have a feeling the setup is more complicated than I require. If someone wants to write an answer explaining how it works, that would be welcome, but please first read through my specific goals and requirements to see if it fits.

- [user docs](https://llvm.org/docs/HowToBuildWithPGO.html)
- [github folder with readme about 2-stage PGO build](https://github.com/llvm/llvm-project/tree/main/clang/cmake/caches)

It will probably irk you to know that I'm asking this question without having investigated how llvm does it in order to learn from it. In my defense, (blaming it on my naivete,) I didn't know what approach to use when doing my googling to learn about this. It was only after I had made a big effort to come up with what I have of a solution so far that I thought to try searching about specific, large, open-source projects. Even now that I know it exists, I feel very intimidated by how much bigger the llvm CMake config setup is than my own project. I know that's not a great reason, but have some mercy please.

### Here are all the specifics of what I'm looking for:

- For the sake of narrowing the scope, I want to use the _instrumentation_ approach to PGO where one creates an instrumented binary and does a training run. Ie. Do not give an answer which uses the _sampling_ approach to PGO.

- Supports all of GCC 11, Clang 14, and MSVC 19

- I only care about using PGO to create a speed-optimized (not space-optimized) release build. I have zero need to ever use PGO for a debug build.
  - Therefore, the approach should probably not be one of providing an `option` cache variable that "adds PGO" to all configs in a generated buildsystem.
  - Instead, I would rather have a single build type/config option specifically for a speed-optimized release build which can be one of the configs in a multi-config buildsystem.

- The answer should not _require_ me to manually perform commandline commands to build the instrumented version of the library or run the training program. I should be able to invoke a single action to build the optimized library, and the invoked action should be able to automatically detect if the instrumented library needs to be rebuilt and/or the training program to be re-run.
  - It would, however, be a plus if it provided a way for me to easily re-run the training step whenever I want (which should also automatically detect if the instrumented library or trainer needs to first be built/rebuilt).

- Nothing should be automatically re-performed unless it has to be. This includes: compiling and linking the training/profiling version of the PGO target, re-running the trainer program, and compiling and linking the optimized version of the PGO target.

- In creating a target for the instrumented version of the library and a target for the optimized version of the library, it should be on the priority list to avoid duplication of CMake code and potential for accidentally skipping/forgetting setting some setting on both of them. It doesn't have to be the top priority, but a great answer would describe alternative approaches and how it thinks they compare with regard to CMake code duplication.

- Avoids having build artifacts/byproducts from the profiling/training and optimizing versions of the builds overwrite each other.

#### I don't need to know

- The general steps of what compiler commands to invoke and in what order and with what arguments to compile a simple program. I have the compiler docs and C++ conference videos for that.

#### I do need to know

- About specific gotchas in the process of invoking the compiler/linker commands that have to do with implementation details of the tools and buildsystem setup that aren't immediately obvious from reading the docs or watching short tutorial videos with toy examples- particularly if it has to do with having object files for the profile and optimize versions of the target exist simultaneously in different directories.

### Links to docs that I have been reading

- [gcc docs on PGO](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html#Instrumentation-Options)
- [clang docs on PGO](https://clang.llvm.org/docs/UsersManual.html#profile-guided-optimization)
- [msvc docs on PGO](https://docs.microsoft.com/en-us/cpp/build/profile-guided-optimizations)

### Why I (think you might want to, but) hope you don't flag this question as being too broad

Yes, I know this is a big ask- particularly the part about supporting three compilers. A sufficient answer is going to be a beast. I have been reading the compiler docs. I am aware that each compiler has different flags and procedures for doing instrumented PGO, but please bear with me. I don't want an answer that supports only a subset of them. I believe that "the bigger picture" I am requesting here is an essential part of the question.

1. Being able to support multiple compilers is a core feature/goal of CMake and its usage.

2. Supporting those three compilers is a core goal of my library. I would like to be faithful to that with respect to supporting PGO if possible.

3. I want people who are looking for help creating such a setup to not have to search for / finally stumble upon five (or however many) different SO questions and then have to piece them together like a homicide detective to figure out the final answer.

A split into multiple questions is fine as long as it doesn't make it inconvenient to piece things back together.

### Existing related questions and why I don't think I'm a duplicate

- [How to express PGO dependencies in CMake](https://stackoverflow.com/questions/42125009/how-express-pgo-dependencies-in-cmake-3-7)
  - This question is very well written, but only for clang.
  - Its main focus is only on the part about making the target that consumes the profile data to re-compile its sources when the profile data changes. I want the full picture.
  - It poses theories about how to fill the gaps it needs, but knows that those theories have problems.
  - The asker's setup defines the target-to-be-trained twice in the same generated buildsystem and doesn't raise any concern about CMake code duplication.
  - There is currently no answer to this question. Update: I added one describing my own approach to solving that problem.

- [CMake compile a program twice in a row](https://stackoverflow.com/questions/45267352/cmake-compile-a-program-twice-in-a-row)
  - That question is g++-only and comparatively lacks detail. Its focus on mostly on creating two similar targets. I require three compilers and have several other detailed requirements.

### My current approach and progress

I have some setup working for GCC, but not yet for clang and MSVC. Notably, MSVC's profile data format and usage workflow seem different so I expect to have to make adjustments.

I use the top-level project for the optimized build and add a copy of the project as an `ExternalProject` and pass it almost all the same configuration options, but set the build type / config types to just be a custom type for the training/profiling build. I used `ExternalProject` instead of creating a duplicate target in the same buildsystem because I'm not aware of any easy way in CMake to duplicate targets, and to manually make sure two targets have everything necessary in common sounded like a readability and opportunity-for-human-error nightmare. I can't say for sure that I've come out on top with the `ExternalProject` approach, but I don't feel unsatisfied with it either. Unlike would probably be the case with the duplicate-target approach, most/all of the boilerplate-y code with the `ExternalProject` approach can be isolated to one `.cmake` file.

What I've found challenging with this approach is that I want to have only some configs (in a single generated multi-config buildsystem) for PGO and some for the regular Debug, Release, RelWithDebInfo, MinSizeRel configs. The challenge comes because many of the mechanisms I use or wanted to use don't support generator expressions, which are used to do things just for specific configs. For example, I can't add an `ExternalProject` based on a generator expression. Currently, [`add_dependencies` doesn't support generator expressions either](https://gitlab.kitware.com/cmake/cmake/-/issues/19467). [Nor does `OBJECT_DEPENDS`](https://gitlab.kitware.com/cmake/cmake/-/issues/22034).

- I have to _always_ add the `ExternalProject`, and add a check to my top-level CMakeLists.txt to immediately `return()` if it has been passed cache variable options that indicate it's being called as an `ExternalProject` by the `PgoUse` config.
- I have to _always_ `add_dependencies` to the custom target and wrap its _`COMMAND`_ with a generator expression to do a cross-platform no-op command if the config isn't `PgoUse`, and instead of being able to use `OBJECT_DEPENDS` with a generator expression.
- I have the custom target script which runs the trainer program create a stamp header file when it ran the training program to produce new data and conditionally include that header into all my source files, relying on the generated buildsystem's header dependency mechanisms to decide that a rebuild is needed.

In terms of compiler things, I found that for GCC, I have to use `-fprofile-prefix-path` to normalize names of profile data files, which by default use absolute paths of object files. I have yet to try out clang or MSVC to see if I have to do anything similar for them.

Some interesting (and frustrating!) things I learned about using `ExternalProject` are:

- That it has many use-cases, and the way it checks whether a re-build is needed is appropriately determined by each way it does or doesn't download the project. When you don't download the project and just provide it as-is from the filesystem, if you plan to manually edit the sources, you need to tell CMake that it should always rebuild no-matter what when anything depends on the build step.
- Now that the build is always re-run when anything depends on it, the stamp file that CMake creates to determine if anything that depends on the build step should be re-run as well is _always_ dirty. The generated buildsystem for the `ExternalProject` may know when it is invoked that nothing has changed and nothing needs to be re-built, but that won't be reflected by CMake's stamp file for the build step. Ie. When I `add_dependencies` the custom target to the build step of the `ExternalProject`, the custom target's command will _always_ be re-run, so the custom target's `COMMAND` needs to roll the logic for the actual check to see if the binaries built by the build step have changed and to short-circuit if the training doesn't need to be re-run. It's not a big deal- just a little painful.

One might ask at this point why I don't just bite the bullet and require generating a buildsystem dedicated to PGO that just uses a single-config generator or tells a multi-config-only generator such as Visual Studio to only generate a config for PGO... I suppose I could. To a user of my project, it would probably make no difference. But to me, as a developer of my project, having a single generated buildsystem for all my configs is convenient and results in less combinatorial explosion of my CMakePreset files. I'm a curious, somewhat idealistic, and stubborn person. If it's just up to me to decide (which it is, since this is a personal project) and I want to try something, even if it will likely only amount to a small benefit, I'll try it.

### My CMake config code

The CMake code I have is free software licensed under `GPL-3.0-or-later`. [You can find the repo here](https://github.com/david-fong/okiidoku/tree/main/cpp), [and the bulk of the relevant CMake config here](https://github.com/david-fong/okiidoku/blob/main/cpp/cmake/okiidoku/pgo_helpers.cmake). I'm hesitant to paste it onto StackOverflow because it seems that [anything posted here is `CC-BY-SA`](https://stackoverflow.blog/2010/08/11/defending-attribution-required/). Perhaps I am being naive and paranoid. I don't even know whether it makes sense / is allowed to put buildsystem configs under the GPL (I'm a complete noob trying slowly to learn when it comes to the legal aspects of software), but that's a different question.

For the time being, please do not give an answer built _on top_ of my project's CMake config without my permission. If you find that problematic, please contact me to discuss.