# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:words emcmake embind emcc
# see emcmake implementation in ${EMSDK}/upstream/emscripten/emcmake.py
#  and ${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
if(NOT EMSCRIPTEN)
	message(FATAL_ERROR "missing emscripten toolchain file. see `emcmake`")
endif()
# TODO.high add an assertion/warning about compatible/tested emsdk version

# https://emscripten.org/docs/compiling/Building-Projects.html#using-libraries
# https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#binding-libraries
add_executable(okiidoku_js okiidoku.embind.cpp)
set_target_properties(okiidoku_js PROPERTIES
	OUTPUT_NAME "okiidoku"
	SUFFIX ".js" # not explicitly needed. defaulted CMAKE_EXECUTABLE_SUFFIX in toolchain file
)
# https://emsettings.surma.technology/
# https://github.com/emscripten-core/emscripten/blob/main/src/settings.js
# https://developer.chrome.com/blog/faster-wasm-debugging/
target_compile_options(okiidoku_js PRIVATE -sSTRICT=1)
target_link_options(okiidoku_js PRIVATE
	--no-entry # it's a "reactor" (ie. a library)
	# --pre-js=
	-sSTRICT=1
	"$<$<CONFIG:Debug,RelWithDebInfo>:-sASSERTIONS=1>"
	"$<$<CONFIG:Debug,RelWithDebInfo>:-gsource-map>" # https://emscripten.org/docs/tools_reference/emcc.html#emcc-gsource-map

	# https://emscripten.org/docs/api_reference/module.html#module
	-sMODULARIZE=1
	-sEXPORT_NAME=OKIIDOKU_CREATE_MODULE
	# -sWASM_ASYNC_COMPILATION=1 this is the default
	# -sINCOMING_MODULE_JS_API=[onRuntimeInitialized]

	# https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#typescript-definitions
	--emit-tsd "$<TARGET_FILE_DIR:okiidoku_js>/okiidoku.d.ts"

	"$<$<CONFIG:Release,MinSizeRel>:--closure=1>" # https://emscripten.org/docs/site/glossary.html#term-closure-compiler

	-sALLOW_UNIMPLEMENTED_SYSCALLS=1 # TODO.wait workaround. why needed?
	# -sSTANDALONE_WASM=1
	# TODO.investigate what is -sMEMORY64 ? (associated with -sWASM_BIGINT=1, which is enabled by default)
	# -sENVIRONMENT=web # use to exclude commonJS/Node glue-code
	# -sMIN_NODE_VERSION
)
okiidoku_add_compiler_options(okiidoku_js)
# TODO: look into emscripten issue ticket #23035 about SIMD targetting
target_link_libraries(okiidoku_js
	PRIVATE okiidoku
	PRIVATE embind
)
okiidoku_install_target(okiidoku_js)

# TODO.wait https://gitlab.kitware.com/cmake/cmake/-/issues/20745
include(GNUInstallDirs)
set(src "$<TARGET_FILE_DIR:okiidoku_js>")
set(dest "${CMAKE_CURRENT_SOURCE_DIR}/examples/node/build/$<CONFIG>")
install(FILES "${src}/okiidoku.d.ts" TYPE BIN COMPONENT okiidoku_development)
install(FILES "${src}/okiidoku.wasm" TYPE BIN COMPONENT okiidoku_runtime)
install(DIRECTORY examples TYPE DOC COMPONENT okiidoku_development
	PATTERN "*.*js"        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
	PATTERN "build"        EXCLUDE
	PATTERN "node_modules" EXCLUDE
	PATTERN "*pnpm*"       EXCLUDE
)
# make a symlink so the examples can use prettier paths.
# too bad can't use execute_process or file(CREATE_LINK) because there's a generator expression in CMAKE_LIBRARY_OUTPUT_DIRECTORY.
set(c "${CMAKE_COMMAND};-E")
add_custom_command(TARGET okiidoku_js POST_BUILD
	COMMAND "${c};make_directory;${dest}"
	COMMAND "${c};create_symlink;$<TARGET_FILE:okiidoku_js>;${dest}/$<TARGET_FILE_NAME:okiidoku_js>"
	COMMAND "${c};create_symlink;${src}/okiidoku.d.ts;${dest}/okiidoku.d.ts"
	COMMAND "${c};create_symlink;${src}/okiidoku.wasm;${dest}/okiidoku.wasm"
	COMMENT "Symlinking JS+WASM+TS target output files to examples directory for convenience"
	VERBATIM COMMAND_EXPAND_LISTS
)