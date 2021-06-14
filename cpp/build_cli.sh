#!/bin/bash
# https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
readonly ROOT="$(dirname "${BASH_SOURCE[0]}")"
# readonly ROOT="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

# if [[ "$@" =~ '--clang' ]]
# then
# 	readonly COMPILER='clang++ -target x86_64-w64-mingw64 -fcolor-diagnostics -ferror-limit=5'
# else
	# Use g++ by default.
	readonly COMPILER='g++ -std=c++20 -pthread -fdiagnostics-color=always -fmax-errors=5'
# fi

# readonly CPP=$(find src -type f -name '*.cpp')
readonly CPP="$(echo "$ROOT/src/solvent_"{\
util/str,\
lib/grid,\
lib/print,\
lib/gen/path,\
lib/gen/mod,\
lib/gen/batch,\
lib/toolkit/mod,\
cli/repl,\
cli/main\
}.cpp)"

# -Wsign-conversion
${COMPILER} \
	-isystem "${ROOT}/src" \
	-Wall -Wextra -Wpedantic -Werror \
	-Wold-style-cast -Wimplicit-fallthrough=5 \
	-march=native -flto \
	"$@" \
	${CPP} -o build/solvent_cli.exe
