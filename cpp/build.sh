#!/bin/bash
#
# Personal notes:
# https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
set -e
readonly CONTEXT="$(dirname "${BASH_SOURCE[0]}")"
#declare -rx GMON_PREFIX="${CONTEXT}/.profile" # this doesn't seem to be working...

if [[ "$@" =~ '--clang' ]]
then
    readonly COMPILER='clang++ -target x86_64-w64-mingw64 -fcolor-diagnostics -fansi-escape-codes -ferror-limit=5'
else
    # Use g++ by default.
    readonly COMPILER='g++ -std=c++17 -pthread -fdiagnostics-color=always -fmax-errors=5'
fi

declare OPTS=''
if [[ "$@" =~ '-p'|'--profile' ]]
then
    echo -e '\nbuilding with profiling support.'
    OPTS+=" -pg -fprofile-generate"
elif [[ "$@" =~ '-P'|'--profile-use' ]]
then
    echo -e '\nusing last generated profiling data.'
    OPTS+=" -fprofile-use"
fi

# TODO [feat] This only designed for windows. Make it work on linux too?
declare -r SHORTEN_STDLIB_PATH='s/.:\/[^\n]+?\/include/\[1;35mSTDLIB\[39m/g'

${COMPILER}\
    -Wall -Wimplicit-fallthrough=5 -O3 ${OPTS}\
    -fprofile-dir="${CONTEXT}/.profile"\
    src/main.cpp -o sudoku.exe\
    |& sed -E ${SHORTEN_STDLIB_PATH}
