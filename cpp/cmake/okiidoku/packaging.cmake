# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:words shlibdeps
include_guard(DIRECTORY)
include(okiidoku/installation)

# Packaging support
# https://cmake.org/cmake/help/book/mastering-cmake/chapter/Packaging%20With%20CPack.html
# https://cmake.org/cmake/help/latest/module/CPack.html
# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#creating-packages
# https://cmake.org/cmake/help/latest/manual/cpack-generators.7.html
set(CPACK_VERBATIM_VARIABLES YES)
set(CPACK_PACKAGE_VENDOR "david-fong")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/../LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/readme.md")
#set(CPACK_PACKAGE_ICON )
set(CPACK_SOURCE_IGNORE_FILES
	# see https://github.com/CLIUtils/CLI11/blob/main/CMakeLists.txt for reference
	[[/\.DS_Store$]] [[/\.git/]] [[/\.git$]] [[\.swp$]] [[~$]]
	[[/\.vscode/]] [[/external/]] [[/out/]] [[/build/]]
	[[/cpp-notes.md]] [[/TODO.md]]
)
set(CPACK_SOURCE_GENERATOR "TGZ")
# TODO.low https://reproducible-builds.org/docs/archives/


# https://decovar.dev/blog/2021/09/23/cmake-cpack-package-deb-apt/
# https://cmake.org/cmake/help/latest/cpack_gen/deb.html#variable:CPACK_DEBIAN_PACKAGE_SHLIBDEPS
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)


include(CPack)