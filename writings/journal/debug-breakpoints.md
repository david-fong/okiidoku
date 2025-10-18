# 25/10/17

So at some recent point, my breakpoints stopped being bound in VS Code. And paths in comiler diagnostics started being relative paths, which meant I couldn't `ctrl+click` in terminal emulators to open the file (but somehow the Problems panel in VS Code / the $gcc pattern matcher or whatever was involved could understand the paths... that I don't know how).

But since I have a (bad) habit of changing a bunch of stuff at once, and I also did an Ubuntu upgrade which led to compiler upgrade, I had no idea what was causing it. I went back a little in Git commit history, but not far enough back to figure out what was the cause, and I thought it might be the toolchain upgrade, but it was happening for both GCC and Clang, and only GCC upgraded with the OS upgrade.

Spoiler alert- it was probably commit `65f6994`, where I uncommented the `hash_dir` config point for ccache from the `build_tools.cmake` file. sigh.

The pieces to understand what's going on are:

- https://sourceware.org/gdb/current/onlinedocs/gdb.html/Source-Path.html
  - https://sourceware.org/gdb/current/onlinedocs/gdb.html/Source-Path.html#set-substitute_002dpath
- https://code.visualstudio.com/docs/cpp/launch-json-reference#_sourcefilemap
- https://ccache.dev/manual/4.11.2.html

```cmake
if(NOT MSVC)
	add_compile_options(
		# "-Werror=date-time" # error to use __TIME__, __DATE__ or __TIMESTAMP__
		"-fno-record-gcc-switches" "-gno-record-gcc-switches"
		"-ffile-prefix-map=${okiidoku_SOURCE_DIR}=/okiidoku"
		#  see also gdb: set substitute-path, lldb: target.source-map, vscode: "sourceFileMap"
		# TODO: why am I still getting absolute paths in debug builds for GCC?
	)
  ...

if(NOT DEFINED CMAKE_CXX_COMPILER_LAUNCHER)
find_program(CCACHE_EXE NAMES ccache DOC "Path to ccache executable")
if(NOT "${CCACHE_EXE}" STREQUAL "CCACHE_EXE-NOTFOUND")
	message("using ccache with namespace `okiidoku.${CMAKE_CXX_COMPILER_ID}.$<CONFIG>`") # (for `ccache --evict-namespace`)
	set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXE}"
		"namespace = okiidoku.${CMAKE_CXX_COMPILER_ID}.$<CONFIG>"
		"ignore_options = /w* /W* -W* -ftabstop=*"
		# man ccache @"COMPILING IN DIFFERENT DIRECTORIES":
		"base_dir = ${okiidoku_SOURCE_DIR}/.."
    "hash_dir = false"
		# "run_second_cpp=false"
	)
```

## Our journey of pain begins

```json
"cmake.debugConfig": {
  // // https://code.visualstudio.com/docs/cpp/launch-json-reference
  "setupCommands": [
    {"text": "directory ${workspaceFolder}/cpp", "ignoreFailures": true,}, // add to source search path. allows loading :/cpp/tools/okiidoku-gdb.py.
    {"text": "set substitute-path /okiidoku ${workspaceFolder}/cpp", "ignoreFailures": true,},
  ],
  "sourceFileMap": {
    "/okiidoku": "${workspaceFolder}/cpp" // since `-ffile-prefix-map=${okiidoku_SOURCE_DIR}=/okiidoku`
  },
},
```

Note that the results here and for all the below sections are essentially the same even without `directory ${workspaceFolder}/cpp`- just that the output of `-exec show directories` doesn't show `:/cpp`.

```gdb
-exec show directories
Source directories searched: /home/david/c/m/ok/cpp:$cdir:$cwd

-exec show substitute-path
List of all source path substitution rules:
  `/okiidoku' -> `/home/david/c/m/ok/cpp'.

-exec info source
Current source file is ../../../libs/okiidoku/tests/test.serdes.cpp
Compilation directory is /okiidoku/out/build/dev.clang
Located in /home/david/c/m/ok/cpp/libs/okiidoku/tests/test.serdes.cpp
Contains 104 lines.
...

-exec info b
Num     Type           Disp Enb Address    What
2       breakpoint     keep y   <PENDING>  /okiidoku/libs/okiidoku/tests/test.serdes.cpp:47
```

VS Code able to resolve source file, but breakpoints not successfully bound:

> Module containing this breakpoint has not yet loaded or the breakpoint address could not be obtained.


## Ohhhh it's because I set `base_dir` for ccache

```cmake
# "base_dir = ${okiidoku_SOURCE_DIR}/.." # commented out
```

```json
{"text": "set substitute-path /okiidoku ${workspaceFolder}/cpp", "ignoreFailures": true,},
"sourceFileMap": {
	"/okiidoku": "${workspaceFolder}/cpp" // since `-ffile-prefix-map=${okiidoku_SOURCE_DIR}=/okiidoku`
},
```

```gdb
-exec show directories
Source directories searched: /home/david/c/m/ok/cpp:$cdir:$cwd

-exec show substitute-path
List of all source path substitution rules:
  `/okiidoku' -> `/home/david/c/m/ok/cpp'.

-exec info source
Current source file is /okiidoku/libs/okiidoku/tests/test.serdes.cpp
Compilation directory is /okiidoku/out/build/dev.clang
Located in /home/david/c/m/ok/cpp/libs/okiidoku/tests/test.serdes.cpp
Contains 104 lines.
...

-exec info b
Num     Type           Disp Enb Address            What
2       breakpoint     keep y   <MULTIPLE>
2.1                         y   0x00005555555c1eae in okiidoku::test::test_serdes<(unsigned char)3>(unsigned long)::{lambda()#1}::operator()[abi:cxx11]() const at /okiidoku/libs/okiidoku/tests/test.serdes.cpp:47
2.2                         y   0x00005555555c8ace in okiidoku::test::test_serdes<(unsigned char)4>(unsigned long)::{lambda()#1}::operator()[abi:cxx11]() const at /okiidoku/libs/okiidoku/tests/test.serdes.cpp:47
2.3                         y   0x00005555555cbdce in okiidoku::test::test_serdes<(unsigned char)5>(unsigned long)::{lambda()#1}::operator()[abi:cxx11]() const at /okiidoku/libs/okiidoku/tests/test.serdes.cpp:47
```

Note that "Current source file" now starts with `/okiidoku` instead of relative path. This is what we expect. See `man ccache`:

> This option should be an absolute path to a directory. If set, ccache will rewrite absolute paths into paths relative to the current working directory, but only absolute paths that begin with base_dir.


## Wait... It's alternatively `sourceFileMap` for VS Code's fault?

```cmake
"base_dir = ${okiidoku_SOURCE_DIR}/.."
```

```json
{"text": "set substitute-path /okiidoku ${workspaceFolder}/cpp", "ignoreFailures": true,},
// "sourceFileMap": {
// 	"/okiidoku": "${workspaceFolder}/cpp" // since `-ffile-prefix-map=${okiidoku_SOURCE_DIR}=/okiidoku`
// },
```

```gdb
-exec show directories
Source directories searched: /home/david/c/m/ok/cpp:$cdir:$cwd

-exec show substitute-path
List of all source path substitution rules:
  `/okiidoku' -> `/home/david/c/m/ok/cpp'.

-exec info source
Current source file is ../../../libs/okiidoku/tests/test.serdes.cpp
Compilation directory is /okiidoku/out/build/dev.clang
Located in /home/david/c/m/ok/cpp/libs/okiidoku/tests/test.serdes.cpp
Contains 104 lines.
...

-exec info b
Num     Type           Disp Enb Address            What
2       breakpoint     keep y   <MULTIPLE>
2.1                         y   0x00005555555c1eae in okiidoku::test::test_serdes<(unsigned char)3>(unsigned long)::{lambda()#1}::operator()[abi:cxx11]() const at ../../../libs/okiidoku/tests/test.serdes.cpp:47
2.2                         y   0x00005555555c8ace in okiidoku::test::test_serdes<(unsigned char)4>(unsigned long)::{lambda()#1}::operator()[abi:cxx11]() const at ../../../libs/okiidoku/tests/test.serdes.cpp:47
2.3                         y   0x00005555555cbdce in okiidoku::test::test_serdes<(unsigned char)5>(unsigned long)::{lambda()#1}::operator()[abi:cxx11]() const at ../../../libs/okiidoku/tests/test.serdes.cpp:47
```

Note the relative path for source file _and_ in breakpoints.


## What if I turn all the debugger knowledge of my prefix-map off?

```cmake
"base_dir = ${okiidoku_SOURCE_DIR}/.."
```

```json
// {"text": "set substitute-path /okiidoku ${workspaceFolder}/cpp", "ignoreFailures": true,},
// "sourceFileMap": {
// 	"/okiidoku": "${workspaceFolder}/cpp" // since `-ffile-prefix-map=${okiidoku_SOURCE_DIR}=/okiidoku`
// },
```

```gdb
-exec show directories
Source directories searched: /home/david/c/m/ok/cpp:$cdir:$cwd

-exec show substitute-path
List of all source path substitution rules:

-exec info source
Current source file is ../../../libs/okiidoku/tests/test.serdes.cpp
Compilation directory is /okiidoku/out/build/dev.clang
Located in /okiidoku/out/build/dev.clang/../../../libs/okiidoku/tests/test.serdes.cpp
...

-exec info b
Num     Type           Disp Enb Address            What
2       breakpoint     keep y   <MULTIPLE>
	breakpoint already hit 1 time
2.1                         y   0x00005555555c1eae in okiidoku::test::test_serdes<(unsigned char)3>(unsigned long)::{lambda()#1}::operator()[abi:cxx11]() const at ../../../libs/okiidoku/tests/test.serdes.cpp:47
2.2                         y   0x00005555555c8ace in okiidoku::test::test_serdes<(unsigned char)4>(unsigned long)::{lambda()#1}::operator()[abi:cxx11]() const at ../../../libs/okiidoku/tests/test.serdes.cpp:47
2.3                         y   0x00005555555cbdce in okiidoku::test::test_serdes<(unsigned char)5>(unsigned long)::{lambda()#1}::operator()[abi:cxx11]() const at ../../../libs/okiidoku/tests/test.serdes.cpp:47
```

Breakpoints work, and GDB itself seems fine, but _VS Code_ (i.e. cpptools?) can't follow along with resolving source files.
Can add breakpoints via `-exec b`, and then show up in `-exec info b`, but they don't show up in the Breakpoints view of VS Code.
Aside: `-exec info source` now doesn't show how many lines the file has?


## What if I tell VS Code but not GDB about the prefix-map?

```cmake
"base_dir = ${okiidoku_SOURCE_DIR}/.."
```

```json
// {"text": "set substitute-path /okiidoku ${workspaceFolder}/cpp", "ignoreFailures": true,},
"sourceFileMap": {
	"/okiidoku": "${workspaceFolder}/cpp" // since `-ffile-prefix-map=${okiidoku_SOURCE_DIR}=/okiidoku`
},
```

```gdb
-exec show directories
Source directories searched: /home/david/c/m/ok/cpp:$cdir:$cwd

-exec show substitute-path
List of all source path substitution rules:

-exec info source
Current source file is ../../../libs/okiidoku/tests/test.serdes.cpp
Compilation directory is /okiidoku/out/build/dev.clang
Located in /okiidoku/out/build/dev.clang/../../../libs/okiidoku/tests/test.serdes.cpp
...

-exec info b
Num     Type           Disp Enb Address    What
2       breakpoint     keep y   <PENDING>  /okiidoku/libs/okiidoku/tests/test.serdes.cpp:47
```

VS Code can resolve source files, but breakpoints aren't bound.
Aside: `-exec info source` also doesn't show how many lines the source file has here... seems like when GDB doesn't have `substitute-path`, this is what happens? But now sure what consequences there are to debugging experience other than `-exec info source` output.

If I add `{"text": "directory ${command:cmake.buildDirectory}", "ignoreFailures": true,},` to this setup, I get-
```gdb
Located in /home/david/c/m/ok/cpp/libs/okiidoku/tests/test.serdes.cpp
```

## Now what

I've decided for now to keep `base_dir` for ccache, remove `sourceFileMap` from settings.json, and add `absolute_paths_in_stderr=true` for ccache so I can `ctrl+click` on my diagnostic messages again (finally).

T^T