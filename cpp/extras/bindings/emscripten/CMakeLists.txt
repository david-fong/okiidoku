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
	LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/emcc_pre_js.js
)
# https://emsettings.surma.technology/
# https://github.com/emscripten-core/emscripten/blob/main/src/settings.js
target_compile_options(okiidoku_js PRIVATE
	-sSTRICT=1
)
target_link_options(okiidoku_js PRIVATE
	--no-entry # it's a "reactor" (ie. a library)
	--pre-js=${CMAKE_CURRENT_SOURCE_DIR}/emcc_pre_js.js

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