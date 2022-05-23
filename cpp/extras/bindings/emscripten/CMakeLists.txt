cmake_minimum_required(VERSION 3.21)
project(okiidoku_emscripten
	VERSION 0.0.1
	DESCRIPTION "emscripten wrapper for libokiidoku"
	HOMEPAGE_URL "https://github.com/david-fong/okiidoku"
	LANGUAGES CXX
)
if(NOT DEFINED EMSCRIPTEN)
	message(FATAL_ERROR "call cmake wrapped with emcmake like `emcmake cmake <...>`")
	# The emcmake wrapper adds command-line arguments to set toolchain file path and such.
endif()
set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

if(NOT DEFINED okiidoku_SOURCE_DIR)
	set(okiidoku_SOURCE_DIR "../../../")
endif()

# You will need conan to fetch private dependencies required to build the library.
# set(OKIIDOKU_CONAN_DEPENDENCIES range-v3/0.11.0)
# include(../../../cmake/ConanPrivateDependencies.cmake)

include(${okiidoku_SOURCE_DIR}/cmake/FetchContent.cmake)

include(${okiidoku_SOURCE_DIR}/cmake/BasicCompileOptions.cmake)

if(okiidoku_IS_TOP_LEVEL)
	include(${okiidoku_SOURCE_DIR}/cmake/internal/StaticAnalyzers.cmake)
	include(${okiidoku_SOURCE_DIR}/cmake/internal/Sanitizers.cmake)
	include(CTest) # https://cliutils.gitlab.io/modern-cmake/chapters/testing.html
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# dynamic libs have runtime overhead
# https://emscripten.org/docs/compiling/Dynamic-Linking.html
set(BUILD_SHARED_LIBS FALSE)

# Note: can't use CMake imports since emcsripten needs to build it from scratch.
add_subdirectory(${okiidoku_SOURCE_DIR}/libs/okiidoku okiidoku)

add_library(okiidoku_emscripten
	okiidoku.embind.cpp
)
target_link_libraries(okiidoku_emscripten
	PRIVATE okiidoku_compile_options_private
	PUBLIC okiidoku
)
