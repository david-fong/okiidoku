#!/bin/sh
# https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
set -e
readonly ROOT="$(dirname "$0")"

# if [[ "$@" =~ '--clang' ]]
# then
# 	readonly COMPILER='clang++ -target x86_64-w64-mingw64 -fcolor-diagnostics -fansi-escape-codes -ferror-limit=5'
# else
	# Use g++ by default.
	readonly COMPILER='g++ -std=c++20 -pthread -fdiagnostics-color=always -fmax-errors=5'
# fi

# readonly CPP=$(find src -type f -name '*.cpp')
readonly CPP='
	src/main.cpp
	src/cli/trials.cpp
	src/cli/repl.cpp
	src/lib/gen/path.cpp
	src/lib/gen/mod.cpp
	src/lib/gen/batch.cpp
'

${COMPILER}\
	-Wall -Werror -Wpedantic -Wimplicit-fallthrough=5 -O3\
	${CPP} -o solvent.exe\
