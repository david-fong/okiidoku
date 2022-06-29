cmake_minimum_required(VERSION 3.21)
project(okiidoku
	VERSION 0.0.1
	DESCRIPTION "david fong's variable-grid-size sudoku toolkit"
	HOMEPAGE_URL "https://github.com/david-fong/okiidoku"
	LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)


include(cmake/compile_opts.basic.cmake)

if(okiidoku_IS_TOP_LEVEL)
	include(cmake/if_top_level/compile_opts.non_portable.cmake)
	include(cmake/if_top_level/static_analyzers.cmake)
	include(cmake/if_top_level/sanitizers.cmake)
	include(CTest) # https://cliutils.gitlab.io/modern-cmake/chapters/testing.html

	include(CheckIPOSupported)
	check_ipo_supported(RESULT CMAKE_INTERPROCEDURAL_OPTIMIZATION)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
endif()

include(cmake/detect_target_isa_support.cmake)


# General use cases should use BUILD_SHARED_LIBS to choose whether to build
# the library as shared or static. okiidoku_SHARED_LIBS should only be used
# for convenience when it is desired to make an exception just for okiidoku.
#
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
include(cmake/shared_lib_things.cmake)


include(cmake/dependencies.cmake)

add_subdirectory(libs/cli_utils)
add_subdirectory(libs/okiidoku)

add_subdirectory(apps/cli)

if(okiidoku_IS_TOP_LEVEL)
	add_subdirectory(tools/okiidoku_translator EXCLUDE_FROM_ALL)
endif()


install(TARGETS
	okiidoku okiidoku_cli
	EXPORT okiidoku_installation_targets
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

# regular installation import support:
install(EXPORT okiidoku_installation_targets
	NAMESPACE okiidoku::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/okiidoku
	FILE okiidoku-${OKIIDOKU_LIB_TYPE_NAME}-targets.cmake
	COMPONENT okiidoku_development
)
# also support importing from this repo's build-tree (no installation):
# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#exporting-targets-from-the-build-tree
export(EXPORT okiidoku_installation_targets
	NAMESPACE okiidoku::
	FILE ${CMAKE_CURRENT_BINARY_DIR}/cmake/okiidoku-targets.cmake
)


# Packaging support
# https://cmake.org/cmake/help/latest/module/CPack.html
# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#creating-packages
set(CPACK_PACKAGE_VENDOR "david-fong")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
set(CPACK_SOURCE_GENERATOR "TGZ")
# include(CPack) # disabled since currently using GCC's "-march=native".