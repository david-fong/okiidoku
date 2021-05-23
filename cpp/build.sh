#!/bin/sh
# https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
set -e
# readonly CONTEXT="$(dirname "${BASH_SOURCE[0]}")"

# if [[ "$@" =~ '--clang' ]]
# then
# 	readonly COMPILER='clang++ -target x86_64-w64-mingw64 -fcolor-diagnostics -fansi-escape-codes -ferror-limit=5'
# else
	# Use g++ by default.
	readonly COMPILER='g++ -std=c++20 -pthread -fdiagnostics-color=always -fmax-errors=5'
# fi

${COMPILER}\
	-Wall -Wimplicit-fallthrough=5 -O3\
	src/main.cpp -o solvent.exe\
