# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)
include(GNUInstallDirs)

# set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
# ^tried in last-ditch attempt to support DLLs. It didn't work probably because I don't know what I'm doing.

if (NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET AND NOT DEFINED CMAKE_VISIBILITY_INLINES_HIDDEN)
	# use strict settings unless already specified by the user.
	set(CMAKE_CXX_VISIBILITY_PRESET hidden)
	set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)
endif()

# https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
# https://docs.conan.io/en/latest/howtos/manage_shared_libraries/rpaths.html
# see `man ld`. `$ORIGIN` and `@executable_path` expand to directory containing the program that contains the directive
# https://itwenty.me/posts/01-understanding-rpath/ and CMP0042
file(RELATIVE_PATH bin_to_lib_relpath "${CMAKE_INSTALL_FULL_BINDIR}" "${CMAKE_INSTALL_FULL_LIBDIR}")
if(APPLE) # when the target system is an Apple platform
	list(APPEND CMAKE_INSTALL_RPATH "@executable_path/${bin_to_lib_relpath}")
elseif(UNIX) # when target system is UNIX
	list(APPEND CMAKE_INSTALL_RPATH "\$ORIGIN/${bin_to_lib_relpath}")
endif()
unset(bin_to_lib_relpath)