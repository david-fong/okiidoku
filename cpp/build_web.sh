#!/bin/bash
readonly WEB_ROOT="$(dirname "${BASH_SOURCE[0]}")"
readonly CPP_ROOT="$(dirname "${BASH_SOURCE[0]}")"

readonly COMPILER='em++ -std=c++20 -fcolor-diagnostics -fansi-escape-codes -ferror-limit=5'

readonly CPP="$(echo "${CPP_ROOT}/src/"{\
lib/gen/batch\
lib/gen/mod,\
lib/gen/path,\
web/main,\
}.cpp)"

${COMPILER}\
	-iquote "${CPP_ROOT}/include"\
	-Wall -Werror -Wpedantic -Wimplicit-fallthrough -O3\
	"$@"\
	${CPP} -o "${WEB_ROOT}/build/web/index.html"\
