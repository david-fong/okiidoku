# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:words emcmake embind emcc
if(NOT DEFINED EMSCRIPTEN)
	message(FATAL_ERROR "missing emscripten toolchain file. see `emcmake`")
endif()

# https://emscripten.org/docs/compiling/Building-Projects.html#using-libraries
# https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#binding-libraries
add_executable(okiidoku_js okiidoku.embind.cpp)
set_target_properties(okiidoku_js PROPERTIES
	OUTPUT_NAME "okiidoku"
	# SUFFIX ".js" # not explicitly needed.
)
# https://emsettings.surma.technology/
# https://github.com/emscripten-core/emscripten/blob/main/src/settings.js
# TODO.wait typescript .d.ts generation https://github.com/emscripten-core/emscripten/issues/7083
target_compile_options(okiidoku_js PRIVATE
	-sSTRICT=1
)
target_link_options(okiidoku_js PRIVATE
	--no-entry # it's a "reactor" (ie. a library)
	# --pre-js=
	-sSTRICT=1
	-sMODULARIZE=1 # https://emscripten.org/docs/api_reference/module.html#module
	-sEXPORT_NAME=okiidoku_create
	# -sINCOMING_MODULE_JS_API=[onRuntimeInitialized]

	-sALLOW_UNIMPLEMENTED_SYSCALLS=1 # TODO.wait workaround. why needed?
	# -sSTANDALONE_WASM=1
	-sWASM_BIGINT=1
	# -sENVIRONMENT=web # use to exclude commonJS/Node glue-code
)
okiidoku_add_compiler_options(okiidoku_js)
target_link_libraries(okiidoku_js
	PRIVATE okiidoku
	PRIVATE embind
)

# make a symlink so the examples can use prettier paths.
set(examples_dir "${CMAKE_CURRENT_SOURCE_DIR}/examples")
set(js_symlink_dir "${examples_dir}/node/build/$<CONFIG>")
add_custom_command(TARGET okiidoku_js POST_BUILD
	COMMAND "${CMAKE_COMMAND};-E;make_directory;${js_symlink_dir}"
	COMMAND "${CMAKE_COMMAND};-E;create_symlink"
		"$<TARGET_FILE:okiidoku_js>"
		"${js_symlink_dir}/$<TARGET_FILE_NAME:okiidoku_js>"
	VERBATIM COMMAND_EXPAND_LISTS
)
