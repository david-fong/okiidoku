# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:words emcmake embind emcc
# see emcmake implementation in ${EMSDK}/upstream/emscripten/emcmake.py
#  and ${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
if(NOT EMSCRIPTEN)
	message(FATAL_ERROR "missing emscripten toolchain file. see `emcmake`")
endif()
if ("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "${OKIIDOKU_EMSCRIPTEN_MIN_VERSION}")
	message(WARNING "the author of okiidoku doesn't test with versions of Emscripten below ${OKIIDOKU_EMSCRIPTEN_MIN_VERSION}")
	# \internal remove this check if Emscripten is switched to a find_package/FetchContent dependency
endif()

# https://emscripten.org/docs/compiling/Building-Projects.html#using-libraries
# https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#binding-libraries
add_executable(okiidoku_js okiidoku.embind.cpp)
set_target_properties(okiidoku_js PROPERTIES
	OUTPUT_NAME "okiidoku"
	SUFFIX ".mjs" # https://emscripten.org/docs/tools_reference/settings_reference.html#export-es6. EMSDK toolchain defaults to ".js"
	SPDX_LICENSE [[AGPL-3.0-or-later]]
)
set_source_files_properties(okiidoku.embind.cpp PROPERTIES
	OBJECT_OUTPUTS "okiidoku.d.ts;okiidoku.wasm;$<${debug_configs}:okiidoku.wasm.map>"
)
# https://emsettings.surma.technology/
# https://raw.githubusercontent.com/emscripten-core/emscripten/refs/heads/main/src/settings.js
# https://raw.githubusercontent.com/emscripten-core/emscripten/refs/heads/main/docs/emcc.txt
# https://developer.chrome.com/blog/faster-wasm-debugging/
target_link_options(okiidoku_js PRIVATE
	--no-entry # it's a "reactor" (ie. a library)
	# --pre-js=
	"$<${debug_configs}:-sASSERTIONS=2>"

	# https://emscripten.org/docs/api_reference/module.html#module
	-sEXPORT_NAME=OKIIDOKU_CREATE_MODULE
	# -sWASM_ASYNC_COMPILATION=1 this is the default
	# -sINCOMING_MODULE_JS_API=[onRuntimeInitialized]

	# https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#typescript-definitions
	--emit-tsd "$<TARGET_FILE_DIR:okiidoku_js>/okiidoku.d.mts" # https://www.typescriptlang.org/docs/handbook/modules/reference.html#file-extension-substitution
	# --emit-minification-map "$<TARGET_FILE_DIR:okiidoku_js>/okiidoku.minification_map.txt"
	# --emit-symbol-map

	"$<$<CONFIG:Release,MinSizeRel>:--closure=1>" # https://emscripten.org/docs/site/glossary.html#term-closure-compiler

	# -sALLOW_UNIMPLEMENTED_SYSCALLS=1 # TODO this was a workaround for something in the past (can't remember what). remove this
	# -sSTANDALONE_WASM=1
	# TODO.investigate what is -sMEMORY64 ? (associated with -sWASM_BIGINT=1, which is enabled by default)
	# -sENVIRONMENT=web # use to exclude commonJS/Node glue-code
	# -sMIN_NODE_VERSION
)
okiidoku_add_compiler_options(okiidoku_js)
# TODO: look into emscripten issue ticket #23035 about SIMD targetting
target_link_libraries(okiidoku_js
	PRIVATE pcg::pcg
	PRIVATE okiidoku
	PRIVATE embind
)
okiidoku_install_target(okiidoku_js)

# TODO.wait https://gitlab.kitware.com/cmake/cmake/-/issues/20745
include(GNUInstallDirs)
set(src "$<TARGET_FILE_DIR:okiidoku_js>/$<TARGET_FILE_BASE_NAME:okiidoku_js>")
install(FILES "${src}.wasm"     TYPE BIN COMPONENT okiidoku_runtime)
install(FILES "${src}.d.mts"    TYPE BIN COMPONENT okiidoku_development)
install(FILES "${src}.wasm.map" TYPE BIN COMPONENT okiidoku_development OPTIONAL)
install(DIRECTORY examples TYPE DOC COMPONENT okiidoku_development
	PATTERN "*.*js"        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
	PATTERN "build"        EXCLUDE
	PATTERN "node_modules" EXCLUDE
	PATTERN "*pnpm*"       EXCLUDE
)

# make a symlink so the examples can use prettier paths.
file(CREATE_LINK
	"${CMAKE_BINARY_DIR}/build"
	"${CMAKE_CURRENT_SOURCE_DIR}/examples/node/build"
	SYMBOLIC
)