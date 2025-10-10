# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:words CMAKEDIR
include_guard(DIRECTORY)
include(GNUInstallDirs)
# https://raw.githubusercontent.com/Kitware/CMake/refs/heads/master/Help/dev/experimental.rst

install(FILES ../README.md ../LICENSE.txt TYPE DOC COMPONENT okiidoku_about)
install(DIRECTORY ../LICENSES TYPE DOC COMPONENT okiidoku_about)

# TODO: install any tweaks header provided by user? but how to do that?... should we abandon tweak header as an options format and go for generating it from cmake option variables?

# support importing from this repo's build-tree (no installation):
# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#exporting-targets-from-the-build-tree
export(
	EXPORT okiidoku_targets
	FILE "${PROJECT_BINARY_DIR}/cmake/okiidoku-targets.cmake"
	NAMESPACE "${PROJECT_NAME}::"
	EXPORT_PACKAGE_DEPENDENCIES
)

# CPS / find_package support
# https://youtu.be/Hk4fv4dD0UQ?t=2899
# https://cmake.org/cmake/help/latest/command/install.html#package-info
install(PACKAGE_INFO okiidoku
	EXPORT okiidoku_targets
	VERSION "${okiidoku_VERSION}"
	COMPAT_VERSION "${okiidoku_COMPAT_VERSION}"
	# COMPAT_VERSION "${_OKIIDOKU_COMPAT_VERSION}"
	DEFAULT_TARGETS lib
	DEFAULT_CONFIGURATIONS Release MinSizeRel RelWithDebInfo Debug
)