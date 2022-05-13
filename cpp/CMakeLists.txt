cmake_minimum_required(VERSION 3.21)
project(okiidoku
	VERSION 0.0.1
	DESCRIPTION "david fong's variable-grid-size sudoku utilities"
	HOMEPAGE_URL "https://github.com/david-fong/okiidoku"
	LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

include(cmake/AddBasicCompileOptions.cmake)

if(okiidoku_IS_TOP_LEVEL)
	include(cmake/internal/StaticAnalyzers.cmake)
	include(cmake/internal/Sanitizers.cmake)
	include(CTest) # https://cliutils.gitlab.io/modern-cmake/chapters/testing.html
endif()

if(NOT MSVC)
	add_compile_options(
		-march=native # enable optimizing for the compiling machine.
		# TODO.wait see https://gcc.gnu.org/gcc-12/changes.html#uninitialized
	)
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)


# use ${BUILD_SHARED_LIBS}, or (if specified), ${okiidoku_SHARED_LIBS}.
# DLLs are currently not supported (I had trouble attempting to get it working).
# https://alexreinking.com/blog/building-a-dual-shared-and-static-library-with-cmake.html
if(MSVC)
	set(okiidoku_SHARED_LIBS FALSE)
elseif(okiidoku_IS_TOP_LEVEL)
	set(okiidoku_SHARED_LIBS TRUE)
endif()
if(DEFINED okiidoku_SHARED_LIBS)
	set(BUILD_SHARED_LIBS "${okiidoku_SHARED_LIBS}")
endif()
include(GNUInstallDirs)
include(cmake/SharedLibThings.cmake)


# You will need conan to fetch private dependencies required to build the library.
set(OKIIDOKU_CONAN_DEPENDENCIES range-v3/0.11.0)
# Note: later, I may add CLI11 as a dependency for the cli
include(cmake/ConanPrivateDependencies.cmake)

add_subdirectory(libs/cli_utils)
add_subdirectory(libs/okiidoku)

add_subdirectory(apps/cli)
# add_subdirectory(apps/web)

if(okiidoku_IS_TOP_LEVEL)
	add_subdirectory(tools/okiidoku_translator)
endif()


# Note: I'm not sure if enabling installation is even useful.
# This library is pretty niche. Perhaps it would be better just to
# export from the build tree. https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#exporting-targets-from-the-build-tree
install(TARGETS
	okiidoku okiidoku_cli
	EXPORT okiidokuTargets
	RUNTIME  COMPONENT okiidoku_runtime
	LIBRARY  COMPONENT okiidoku_runtime
	NAMELINK_COMPONENT okiidoku_development
	ARCHIVE  COMPONENT okiidoku_development
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(BUILD_SHARED_LIBS)
	set(OKIIDOKU_LIB_TYPE_NAME shared)
else()
	set(OKIIDOKU_LIB_TYPE_NAME static)
endif()
install(EXPORT okiidokuTargets
	NAMESPACE okiidoku::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/okiidoku
	FILE okiidoku-${OKIIDOKU_LIB_TYPE_NAME}-targets.cmake
	COMPONENT okiidoku_development
)


# Packaging support
# https://cmake.org/cmake/help/latest/module/CPack.html
# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#creating-packages
set(CPACK_PACKAGE_VENDOR "david-fong")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
set(CPACK_SOURCE_GENERATOR "TGZ")
# include(CPack) # disabled since currently using GCC's "-march=native".