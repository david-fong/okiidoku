# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.21)
project(okiidoku_emscripten
	VERSION 0.0.1
	DESCRIPTION "emscripten build of libokiidoku"
	HOMEPAGE_URL "https://github.com/david-fong/okiidoku"
	LANGUAGES CXX
)
if(NOT DEFINED EMSCRIPTEN)
	message(FATAL_ERROR "missing emscripten toolchain file. see `emcmake`")
endif()

set(okiidoku_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../../..")
list(APPEND CMAKE_MODULE_PATH "${okiidoku_SOURCE_DIR}/cmake")

include(okiidoku/compile_opts.basic)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(BUILD_SHARED_LIBS FALSE) # https://emscripten.org/docs/compiling/Dynamic-Linking.html

include(okiidoku/dependencies)

if(PROJECT_IS_TOP_LEVEL)
	include(okiidoku/if_top_level/static_analyzers)
	include(okiidoku/if_top_level/sanitizers)
	enable_testing()
endif()

include(okiidoku/detect_target_isa_support)


# Note: can't use CMake imports since emscripten needs to build it from scratch.
# add_subdirectory(${okiidoku_SOURCE_DIR}/libs/okiidoku okiidoku)
add_subdirectory(${okiidoku_SOURCE_DIR} okiidoku EXCLUDE_FROM_ALL)

# https://emscripten.org/docs/compiling/Building-Projects.html#using-libraries
# https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#binding-libraries
add_executable(okiidoku_js okiidoku.embind.cpp)
set_target_properties(okiidoku_js PROPERTIES
	OUTPUT_NAME "okiidoku"
	LINK_DEPENDS ${PROJECT_SOURCE_DIR}/emcc_pre_js.js
)
# https://emsettings.surma.technology/
# https://github.com/emscripten-core/emscripten/blob/main/src/settings.js
target_compile_options(okiidoku_js PRIVATE
	-sSTRICT=1
)
target_link_options(okiidoku_js PRIVATE
	--no-entry
	--pre-js=${PROJECT_SOURCE_DIR}/emcc_pre_js.js

	-sSTRICT=1
	-sALLOW_UNIMPLEMENTED_SYSCALLS=1 # TODO.wait workaround. why needed?
	# -sSTANDALONE_WASM=1
	-sWASM_BIGINT=1
	# -sMODULARIZE=1
	# -sENVIRONMENT=web # use to exclude commonJS/Node glue-code
)
target_link_libraries(okiidoku_js
	PUBLIC  okiidoku_compile_options_public
	PRIVATE okiidoku_compile_options_private
	PRIVATE okiidoku
	PRIVATE embind
)