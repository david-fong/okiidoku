# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:words CMAKEDIR
include_guard(DIRECTORY)
include(GNUInstallDirs)

install(FILES ../README.md ../LICENSE.txt TYPE DOC COMPONENT okiidoku_about)
# install(FILES ../.reuse/dep5 TYPE DOC COMPONENT okiidoku_about RENAME copying) # hm probably not right. need to make a custom one for the installation
install(DIRECTORY ../LICENSES TYPE DOC COMPONENT okiidoku_about)

# TODO: install any tweaks header provided by user? but how to do that?... should we abandon tweak header and go for generating it from cmake option variables?

block()
if(BUILD_SHARED_LIBS)
	set(lib_type "shared")
else()
	set(lib_type "static")
endif()
install( # regular installation import support:
	EXPORT okiidoku_installation_targets
	NAMESPACE "${PROJECT_NAME}::"
	DESTINATION "${OKIIDOKU_INSTALL_CMAKEDIR}"
	FILE "okiidoku-${lib_type}-targets.cmake"
	COMPONENT okiidoku_development
)
endblock()

# also support importing from this repo's build-tree (no installation):
# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#exporting-targets-from-the-build-tree
export(
	EXPORT okiidoku_installation_targets
	NAMESPACE "${PROJECT_NAME}::"
	FILE "${PROJECT_BINARY_DIR}/cmake/okiidoku-targets.cmake"
)


# find_package support
# https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html#example-generating-package-files
# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#creating-relocatable-packages
include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${okiidoku_SOURCE_DIR}/cmake/okiidoku/okiidoku-config.cmake.in"
	"${okiidoku_BINARY_DIR}/cmake/okiidoku-config.cmake"
	INSTALL_DESTINATION "${OKIIDOKU_INSTALL_CMAKEDIR}"
	# PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR
)
write_basic_package_version_file(
	"${okiidoku_BINARY_DIR}/cmake/okiidoku-config-version.cmake"
	COMPATIBILITY "${_OKIIDOKU_VERSION_COMPATIBILITY}"
)
# TODO.wait: https://cmake.org/cmake/help/latest/command/install.html#package-info
install(FILES
	"${okiidoku_BINARY_DIR}/cmake/okiidoku-config.cmake"
	"${okiidoku_BINARY_DIR}/cmake/okiidoku-config-version.cmake"
	DESTINATION "${OKIIDOKU_INSTALL_CMAKEDIR}"
	COMPONENT okiidoku_development
)
