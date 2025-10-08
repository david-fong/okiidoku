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