cmake_minimum_required(VERSION 3.21)
project(okiidoku_emscripten
	VERSION 0.0.1
	DESCRIPTION "emscripten build of libokiidoku"
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

# https://emscripten.org/docs/compiling/Building-Projects.html#using-libraries
# https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#binding-libraries
add_executable(okiidoku_js okiidoku.embind.cpp)
set_target_properties(okiidoku_js PROPERTIES
	OUTPUT_NAME "okiidoku"
	LINK_DEPENDS ${PROJECT_SOURCE_DIR}/emcc_pre_js.js
)
# https://emsettings.surma.technology/
target_compile_options(okiidoku_js PRIVATE
	-sSTRICT=1
)
target_link_options(okiidoku_js PRIVATE
	-sSTRICT=1
	# -sSTANDALONE_WASM=1
	-sWASM_BIGINT=1
	# -sMODULARIZE=1
	# -sENVIRONMENT=web # use to exclude commonJS/Node glue-code
	--pre-js=${PROJECT_SOURCE_DIR}/emcc_pre_js.js
)
target_link_libraries(okiidoku_js
	PRIVATE okiidoku_compile_options_private
	PRIVATE okiidoku
	PRIVATE embind
)