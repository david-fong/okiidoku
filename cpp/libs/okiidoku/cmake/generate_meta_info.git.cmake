# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:dictionaries cpp-refined
cmake_minimum_required(VERSION 4.0)
if(CMAKE_SCRIPT_MODE_FILE STREQUAL "")
	message(FATAL_ERROR "this should be called as a script")
endif()

find_package(Git QUIET)
if(NOT Git_FOUND)
	# set(OKIIDOKU_GIT_BUILT_FROM_REPO "false")
	set(OKIIDOKU_GIT_REMOTES "")
	set(OKIIDOKU_GIT_BRANCH "")
	set(OKIIDOKU_GIT_COMMIT "")
else()
	# set(OKIIDOKU_GIT_BUILT_FROM_REPO "true")
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" remote -v
		OUTPUT_VARIABLE OKIIDOKU_GIT_REMOTES
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref --symbolic-full-name @{u}
		OUTPUT_VARIABLE OKIIDOKU_GIT_BRANCH
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	execute_process(
		# alternatively, `git rev-parse @` (which doesn't include whether index is dirty)
		COMMAND "${GIT_EXECUTABLE}" describe --always --dirty "--match=\\" --abbrev=40
		OUTPUT_VARIABLE OKIIDOKU_GIT_COMMIT
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif()

set(delim "\"'\"'")
file(CONFIGURE OUTPUT ${OUTPUT} CONTENT
"// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/about.hpp>
namespace okiidoku::about {
	const GitInfo git_info {
		.remotes {R\"${delim}(${OKIIDOKU_GIT_REMOTES})${delim}\"},
		.branch {R\"${delim}(${OKIIDOKU_GIT_BRANCH})${delim}\"},
		.commit {R\"${delim}(${OKIIDOKU_GIT_COMMIT})${delim}\"},
	};
}")
