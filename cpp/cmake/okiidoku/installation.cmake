# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:words CMAKEDIR
include_guard(DIRECTORY)
include(GNUInstallDirs)

install(FILES ../README.md ../LICENSE.txt TYPE DOC COMPONENT okiidoku_about)
# install(FILES ../.reuse/dep5 TYPE DOC COMPONENT okiidoku_about RENAME copying) # hm probably not right. need to make a custom one for the installation
install(DIRECTORY ../LICENSES TYPE DOC COMPONENT okiidoku_about)

block()
if(BUILD_SHARED_LIBS)
	set(lib_type shared)
else()
	set(lib_type static)
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


include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	"${PROJECT_BINARY_DIR}/cmake/okiidoku-config-version.cmake"
	COMPATIBILITY "${OKIIDOKU_VERSION_COMPATIBILITY}"
)
# support for `find_package`. seems complicated to set up and I'm not sure anyone will want this. I don't.
# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#creating-relocatable-packages
# install(FILES
# 	# "${PROJECT_BINARY_DIR}/cmake/okiidoku-config.cmake" # TODO
# 	"${PROJECT_BINARY_DIR}/cmake/okiidoku-config-version.cmake"
# 	DESTINATION "${OKIIDOKU_INSTALL_CMAKEDIR}"
# 	COMPONENT okiidoku_development
# )