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

set(okiidoku_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../../../")

include(${okiidoku_SOURCE_DIR}/cmake/FetchContent.cmake)

include(${okiidoku_SOURCE_DIR}/cmake/BasicCompileOptions.cmake)

if(okiidoku_IS_TOP_LEVEL)
	include(${okiidoku_SOURCE_DIR}/cmake/internal/StaticAnalyzers.cmake)
	include(${okiidoku_SOURCE_DIR}/cmake/internal/Sanitizers.cmake)
	include(CTest) # https://cliutils.gitlab.io/modern-cmake/chapters/testing.html
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)


set(BUILD_SHARED_LIBS FALSE) # https://emscripten.org/docs/compiling/Dynamic-Linking.html

# Note: can't use CMake imports since emcsripten needs to build it from scratch.
add_subdirectory(${okiidoku_SOURCE_DIR}/libs/okiidoku okiidoku)

add_library(okiidoku_emscripten okiidoku.embind.cpp)
target_link_libraries(okiidoku_emscripten
	PRIVATE okiidoku_compile_options_private
	PRIVATE okiidoku
)

# https://emscripten.org/docs/compiling/Building-Projects.html#using-libraries
# https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#binding-libraries
# https://cmake.org/cmake/help/latest/command/add_custom_command.html#build-events
add_custom_command(TARGET okiidoku_emscripten POST_BUILD
	COMMENT "linking okiidoku to produce wasm and js gluecode"
	COMMAND emcc -lembind -o "okiidoku.js" # TODO currently outputs to root of binary dir. build types will overwrite each other. please nest according to build type/config. Make sure consistent between generators (MSVC, Ninja, etc.)
		-Wl,--whole-archive
		$<TARGET_FILE:okiidoku> # Note: this line is needed.
		$<TARGET_FILE:okiidoku_emscripten>
		-Wl,--no-whole-archive
	VERBATIM
)