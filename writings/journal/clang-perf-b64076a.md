25/10/08

for reason I have no understanding of, a clang build of commit `b64076a` with `origin/main`, with `main` pointing one commit ahead is faster than a clang build of a switch to a branch at the following commit, but checking out all the file contents of `b64076a`. like- 15.6s versis 16.0s, to generate 1,000,000 random `Grid<3>`s. :/

clang version info:

```none
Ubuntu clang version 20.1.2 (0ubuntu1)
Target: x86_64-pc-linux-gnu
Thread model: posix
InstalledDir: /usr/lib/llvm-20/bin
```

I wonder if this has anything to do with the `git_info` global variable...

---

so... somehow it was due to the `git_info` global variable.

here's the content when checking out commit `b64076a`, (where `origin/main` is currently pointing):

  1 // SPDX-FileCopyrightText: 2020 me
  2 // SPDX-License-Identifier: AGPL-3.0-or-later
  3 #include <myproject/about.hpp>
  4 namespace myproject::about {
  5 ┊  const GitInfo git_info {
  6 ┊  ┊  .remotes {R""'"'(origin┊git@github.com:me/myproject.git (fetch)
  7 origin┊  git@github.com:me/myproject.git (push))"'"'"},
  8 ┊  ┊  .branch {R""'"'()"'"'"},
  9 ┊  ┊  .commit {R""'"'(<commit-sha>)"'"'"},
 10 ┊  };
 11 }

here's the content when checking out a later commit, and then checking out the _contents_ of `b64076a` with `git checkout origin/main :/`:
```none
  1 // SPDX-FileCopyrightText: 2020 me
  2 // SPDX-License-Identifier: AGPL-3.0-or-later
  3 #include <myproject/about.hpp>
  4 namespace myproject::about {
  5 ┊  const GitInfo git_info {
  6 ┊  ┊  .remotes {R""'"'(origin┊git@github.com:me/myproject.git (fetch)
  7 origin┊  git@github.com:me/myproject.git (push))"'"'"},
  8 ┊  ┊  .branch {R""'"'(origin/main)"'"'"},
  9 ┊  ┊  .commit {R""'"'(<commit-sha>-dirty)"'"'"},
 10 ┊  };
 11 }
```

and somehow that's what makes the difference. if I manually edit the file, I can produce the same change in observable behaviour (execution time). making the variable definition `constinit` or `constexpr` doesn't seem to make a difference. my _guess_ is that somehow it's affecting offset/alignment of other globals that affect performance?

funnily enough, I'd just started considering to not embed the git status info in the binary, and instead distribute it with the library as a standalone text file. (based on reading https://blog.llvm.org/2019/11/deterministic-builds-with-clang-and-lld.html). I guess that's next on the todo list :/