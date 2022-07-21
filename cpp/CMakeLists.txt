cmake_minimum_required(VERSION 3.21)
project(okiidoku
	VERSION 0.0.1
	DESCRIPTION "david fong's variable-grid-size sudoku toolkit"
	HOMEPAGE_URL "https://github.com/david-fong/okiidoku"
	LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(okiidoku/compile_opts.basic)

if(PROJECT_IS_TOP_LEVEL)
	set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
	set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
	set(CMAKE_CXX_EXTENSIONS FALSE)

	include(okiidoku/if_top_level/compile_opts.non_portable)
	include(okiidoku/if_top_level/static_analyzers)
	include(okiidoku/if_top_level/sanitizers)
	enable_testing() # not `include(CTest)` because I don't use CDash.

	include(CheckIPOSupported)
	check_ipo_supported(RESULT CMAKE_INTERPROCEDURAL_OPTIMIZATION)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE) # want fast debug build
endif()

include(okiidoku/detect_target_isa_support)

include(okiidoku/dependencies)


# General use cases should use BUILD_SHARED_LIBS to choose whether to build
# the library as shared or static. okiidoku_SHARED_LIBS should only be used
# for convenience when it is desired to make an exception just for okiidoku.
#
# DLLs are currently not supported (I had trouble attempting to get it working).
# https://alexreinking.com/blog/building-a-dual-shared-and-static-library-with-cmake.html
if(MSVC)
	set(okiidoku_SHARED_LIBS FALSE)
elseif(PROJECT_IS_TOP_LEVEL)
	set(okiidoku_SHARED_LIBS TRUE)
endif()
if(DEFINED okiidoku_SHARED_LIBS)
	set(BUILD_SHARED_LIBS "${okiidoku_SHARED_LIBS}")
	unset(okiidoku_SHARED_LIBS)
endif()
include(GNUInstallDirs)
include(okiidoku/shared_lib_things)


add_subdirectory(libs/okiidoku)
add_subdirectory(libs/cli_utils)
add_subdirectory(apps/cli)
add_subdirectory(tools/okiidoku_translator EXCLUDE_FROM_ALL)
add_subdirectory(extras/bindings/python EXCLUDE_FROM_ALL)
# Note: emscripten bindings require emscripten's toolchain. see its subdir.


include(okiidoku/installation)


# Packaging support
# https://cmake.org/cmake/help/latest/module/CPack.html
# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#creating-packages
set(CPACK_PACKAGE_VENDOR "david-fong")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
set(CPACK_SOURCE_GENERATOR "TGZ")
# include(CPack) # disabled since currently using GCC's "-march=native".