# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:words shlibdeps
include_guard(DIRECTORY)
if(OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE)
	message(NOTICE "note that okiidoku packaging rules are not enabled due to using `OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE`.")
	return()
endif()

# Packaging support
# https://cmake.org/cmake/help/book/mastering-cmake/chapter/Packaging%20With%20CPack.html
# https://cmake.org/cmake/help/latest/module/CPack.html
# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#creating-packages
# https://cmake.org/cmake/help/latest/manual/cpack-generators.7.html
set(CPACK_VERBATIM_VARIABLES YES) # NO is the default for legacy reasons. new projects should use YES.
set(CPACK_PACKAGE_VENDOR "david-fong")
set(CPACK_RESOURCE_FILE_LICENSE "${okiidoku_SOURCE_DIR}/../LICENSE.txt")
set(CPACK_RESOURCE_FILE_README  "${okiidoku_SOURCE_DIR}/../README.md")
# set(CPACK_PACKAGE_INSTALL_DIRECTORY okiidoku)
#set(CPACK_PACKAGE_ICON )
set(CPACK_SOURCE_IGNORE_FILES
	# see https://github.com/CLIUtils/CLI11/blob/main/CMakeLists.txt for reference
	[[/\.DS_Store$]] [[/\.git/]] [[/\.git$]] [[\.swp$]] [[~$]]
	[[/\.vscode/]] [[/external/]] [[/out/]] [[/build/]]
	[[/cpp-notes.md]] [[/TODO.md]]
)
set(CPACK_SOURCE_GENERATOR "TGZ")
# TODO.low https://reproducible-builds.org/docs/archives/
# https://cmake.org/cmake/help/latest/cpack_gen/archive.html


# https://decovar.dev/blog/2021/09/23/cmake-cpack-package-deb-apt/
# https://cmake.org/cmake/help/book/mastering-cmake/chapter/Packaging%20With%20CPack.html#cpack-for-debian-packages
# https://cmake.org/cmake/help/latest/cpack_gen/deb.html#variable:CPACK_DEBIAN_PACKAGE_SHLIBDEPS
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)


include(CPack)