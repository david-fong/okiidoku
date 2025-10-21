# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:dictionaries cpp-refined
cmake_minimum_required(VERSION 4.1)
if(CMAKE_SCRIPT_MODE_FILE STREQUAL "")
	message(FATAL_ERROR "this should be called as an install script")
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
	string(REPLACE "\n" "\n  " OKIIDOKU_GIT_REMOTES "${OKIIDOKU_GIT_REMOTES}")
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

# include(GNUInstallDirs) # want to use DATADIR, but get warning about enabling a language, and `enable_language` also gives a warning :/
file(CONFIGURE OUTPUT "${CMAKE_INSTALL_PREFIX}/share/okiidoku/about.git.md" CONTENT
"<!-- SPDX-FileCopyrightText: 2020 David Fong -->
<!-- SPDX-License-Identifier: CC0-1.0 -->

- git remotes:
  ```none
  ${OKIIDOKU_GIT_REMOTES}
  ```

- git branch:
  ```none
  ${OKIIDOKU_GIT_BRANCH}
  ```

- git commit:
  ```none
  ${OKIIDOKU_GIT_COMMIT}
  ```
")
install(FILES "${CMAKE_INSTALL_PREFIX}/share/okiidoku/about.git.md" TYPE DOC COMPONENT okiidoku_about)