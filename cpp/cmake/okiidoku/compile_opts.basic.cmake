# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:ignoreRegExp -[W][a-z-]+\b
include_guard(DIRECTORY)

add_library(okiidoku_compile_options_public INTERFACE)
add_library(okiidoku_compiler_warnings INTERFACE IMPORTED) # Note: "IMPORTED" used to prevent auto installation
okiidoku_install_target(okiidoku_compile_options_public)

function(okiidoku_make_include_flag file return_var_name)
	# intentionally leaves target_compile_options and target_sources up to the caller.
	# things like wrapping with generator expression(s) is hard to do in a CMake function.
	set(gnu_include  "$<$<CXX_COMPILER_ID:GNU,Clang>:SHELL:-include '${file}'>")
	set(msvc_include "$<$<CXX_COMPILER_ID:MSVC>:SHELL:/FI ${file}>")
	# TODO warn on unsupported compiler?
	set("${return_var_name}" "${gnu_include}${msvc_include}" PARENT_SCOPE)
endfunction()

function(okiidoku_add_compiler_options target)
	target_link_libraries(${target}
		PUBLIC okiidoku_compile_options_public
	)
	if(OKIIDOKU_BUILD_WITH_SUGGESTED_WARNINGS)
		target_link_libraries(${target}
			PRIVATE okiidoku_compiler_warnings
		)
	endif()
endfunction()

set_target_properties(okiidoku_compile_options_public PROPERTIES EXPORT_NAME _compile_options)

# source file and compiler option parsing rules:
# Note: the pragma flags can currently be private since I currently
#  don't use any compiler-specific pragmas in any public header files.
if(MSVC)
	target_compile_options(okiidoku_compile_options_public INTERFACE
		/wd5030 # warning disable: "unrecognized attribute"
	)
	target_compile_options(okiidoku_compiler_warnings INTERFACE
		/options:strict # unrecognized compiler options are errors
		/utf-8  # /source-charset:utf-8 (for preprocessor), and /execution-charset:utf8 (for compiler)
		/wd4068 # warning disable: "unrecognized pragma"
	)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
block()
	set(v12 "$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,12>")
	target_compile_options(okiidoku_compile_options_public INTERFACE
	)
	target_compile_options(okiidoku_compiler_warnings INTERFACE
		-ftabstop=1
		"$<${v12}:-Wbidi-chars=any>" # warn on any usage of bidi text
		-Wnormalized # warn on identifiers that look the same but are not the same
		-Wno-unknown-pragmas
		"$<${v12}:-Wno-attributes=clang::>"
	)
endblock()
endif()


# diagnostics formatting:
# TODO hm. these make more sense to be user-controlled globally-applied flags. maybe should be moved to CMakePresets.json
if(okiidoku_IS_TOP_LEVEL)
	if(MSVC)
		target_compile_options(okiidoku_compiler_warnings INTERFACE
			/diagnostics:caret
		)
	else()
		target_compile_options(okiidoku_compiler_warnings INTERFACE
			-fdiagnostics-parseable-fixits
		)
	endif()
endif()


# standards compliance and abi:
if(MSVC)
	target_compile_options(okiidoku_compiler_warnings INTERFACE
		/permissive-  # https://discourse.cmake.org/t/cxx-extensions-and-permissive/1994
		/volatile:iso # https://docs.microsoft.com/en-us/cpp/build/reference/volatile-volatile-keyword-interpretation#remarks
	)
else()
	target_compile_options(okiidoku_compiler_warnings INTERFACE
		# -Wabi
	)
endif()


# related to reproducible builds / deterministic compilation:
# TODO is this not the wrong place to do this? wouldn't we want this to apply to all targets?
if(MSVC)
	target_compile_options(okiidoku_compiler_warnings INTERFACE
	)
else()
block()
	set(v12 "$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,12>")
	target_compile_options(okiidoku_compiler_warnings INTERFACE
		-Werror=date-time # error to use __TIME__, __DATE__ or __TIMESTAMP__
		-fno-record-gcc-switches -gno-record-gcc-switches
		"-ffile-prefix-map=${okiidoku_SOURCE_DIR}=/okiidoku"
		# TODO.low try ^this out and see what benefits there are.
		#  see also gdb: set substitute-path, lldb: target.source-map, vscode: "sourceFileMap"
		#  and https://github.com/emscripten-core/emscripten/blob/main/ChangeLog.md#406---032625
		#  https://emscripten.org/docs/tools_reference/settings_reference.html#source-map-prefixes
		# TODO: why am I still getting absolute paths in debug builds for GCC?
	)
	# TODO clang -fno-record-command-line
endblock()
endif()


# features (change emitted / possible-emitted code- not just emit diagnostics):
if(MSVC)
else()
	if(OKIIDOKU_BUILD_DEBUG_WITH_SANITIZERS)
		# AddressSanitizer doesn't play well with `_FORTIFY_SOURCE`.
		# see https://github.com/google/sanitizers/wiki/AddressSanitizer#faq
		#  and https://github.com/google/sanitizers/issues/247.
		target_compile_options(okiidoku_compiler_warnings INTERFACE
			"$<${debug_configs}:-U_FORTIFY_SOURCE;-D_FORTIFY_SOURCE=3>"
		)
	endif()
	if((CMAKE_CXX_COMPILER_ID MATCHES [[Clang]]) OR EMSCRIPTEN)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(okiidoku_compiler_warnings INTERFACE
			-fno-nonansi-builtins # Disable built-in declarations of functions that are not mandated by ANSI/ISO C. These include ffs, alloca, _exit, index, bzero, conjf, and other related functions.
			# TODO.try -fno-implicit-templates # https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html#index-fno-implicit-templates
		)
	endif()
endif()


# warnings:
# (for diagnostics that don't change the emitted object files)
if(MSVC)
	target_compile_options(okiidoku_compiler_warnings INTERFACE
		/W4 # highest warnings level
	)
else()
	set(flag_file_dir "${okiidoku_SOURCE_DIR}/cmake/okiidoku/compile_opts")
	# set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${flags_file}")
	target_compile_definitions(okiidoku_compiler_warnings INTERFACE
		"$<${debug_configs}:_GLIBCXX_ASSERTIONS>"
	)
	target_compile_options(okiidoku_compiler_warnings INTERFACE
		-Wfatal-errors # stop compilation on first error. I found it hard to read multiple.
		"@${flag_file_dir}/warnings.gnu.txt"
		# -Warith-conversion
		# -Wframe-larger-than=byte-size -Wstack-usage=byte-size
	)
	if((CMAKE_CXX_COMPILER_ID MATCHES [[Clang]]) OR EMSCRIPTEN)
		target_compile_options(okiidoku_compiler_warnings INTERFACE
			-Wimplicit-fallthrough
			-Wno-unknown-attributes # (I'd like to use this, but it's just too annoying, flagging benign stuff).
		)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(okiidoku_compiler_warnings INTERFACE
			"@${flag_file_dir}/warnings.gcc.txt"

			# -Wunsafe-loop-optimizations # only meaningful with -funsafe-loop-optimizations

			# interesting but probably too overboard:
			# -Wpadded
			# -Wsign-promo # actually warns on overload selection behaviour mandated by the standard :O
		)
	endif()
endif()
