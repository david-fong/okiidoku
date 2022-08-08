# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:words shlibdeps
include_guard(DIRECTORY)
include(okiidoku/installation)

# Packaging support
# https://cmake.org/cmake/help/book/mastering-cmake/chapter/Packaging%20With%20CPack.html
# https://cmake.org/cmake/help/latest/module/CPack.html
# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#creating-packages
set(CPACK_PACKAGE_VENDOR "david-fong")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
#set(CPACK_PACKAGE_ICON )
set(CPACK_SOURCE_GENERATOR "TGZ")


# https://decovar.dev/blog/2021/09/23/cmake-cpack-package-deb-apt/
# https://cmake.org/cmake/help/latest/cpack_gen/deb.html#variable:CPACK_DEBIAN_PACKAGE_SHLIBDEPS
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)


# include(CPack) # disabled since currently using GCC's "-march=native".