# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:ignoreRegExp -[W][a-z-]+\b
include_guard(DIRECTORY)

add_library(okiidoku::compiler_warnings INTERFACE IMPORTED) # TODO.wait could we use INTERFACE SYMBOLIC?
set_target_properties(okiidoku::compiler_warnings PROPERTIES EXPORT_FIND_PACKAGE_NAME okiidoku) # ugly hack to make it not need to be exported

# \param file - what you would want to write in `#include <...>`.
#  it's up to you to set up any supporting include path flags.
# don't add these as INTERFACE compile options on exported targets-
#  it assumes the host compiler's flag style.
function(okiidoku_make_include_flag file return_var_name)
	# intentionally leaves target_compile_options and target_sources up to the caller.
	# things like wrapping with generator expression(s) is hard to do in a CMake function.
	if(MSVC)
		set("${return_var_name}" "SHELL:/FI ${file}" PARENT_SCOPE)
	else() # TODO warn on unsupported compiler?
		set("${return_var_name}" "SHELL:-include '${file}'" PARENT_SCOPE)
	endif()
endfunction()

function(okiidoku_add_compiler_options target)
	if(OKIIDOKU_BUILD_WITH_SUGGESTED_WARNINGS)
		target_link_libraries(${target}
			PRIVATE okiidoku::compiler_warnings
		)
	endif()
endfunction()

# source file and compiler option parsing rules:
# note: the pragma flags can currently be private since I currently
#  don't use any compiler-specific pragmas in any public header files.
if(MSVC)
	target_compile_options(okiidoku::compiler_warnings INTERFACE
		/wd5030 # warning disable: "unrecognized attribute"
	)
	target_compile_options(okiidoku::compiler_warnings INTERFACE
		/options:strict # unrecognized compiler options are errors
		/utf-8  # /source-charset:utf-8 (for preprocessor), and /execution-charset:utf8 (for compiler)
		/wd4068 # warning disable: "unrecognized pragma"
	)
endif()


# diagnostics formatting:
# TODO hm. these make more sense to be user-controlled globally-applied flags. maybe should be moved to CMakePresets.json
if(okiidoku_IS_TOP_LEVEL)
	if(MSVC)
		target_compile_options(okiidoku::compiler_warnings INTERFACE
			/diagnostics:caret
		)
	else()
		target_compile_options(okiidoku::compiler_warnings INTERFACE
			-ftabstop=1
			-fdiagnostics-parseable-fixits
		)
	endif()
endif()


# standards compliance and abi:
if(MSVC)
	target_compile_options(okiidoku::compiler_warnings INTERFACE
		/permissive-  # https://discourse.cmake.org/t/cxx-extensions-and-permissive/1994
		/volatile:iso # https://docs.microsoft.com/en-us/cpp/build/reference/volatile-volatile-keyword-interpretation#remarks
	)
endif()


# features (change emitted / possible-emitted code- not just diagnostics):
if(MSVC)
else()
	if(OKIIDOKU_BUILD_DEBUG_WITH_SANITIZERS)
		# AddressSanitizer doesn't play well with `_FORTIFY_SOURCE`.
		# see https://github.com/google/sanitizers/wiki/AddressSanitizer#faq
		#  and https://github.com/google/sanitizers/issues/247.
		target_compile_options(okiidoku::compiler_warnings INTERFACE
			"$<${debug_configs}:-U_FORTIFY_SOURCE;-D_FORTIFY_SOURCE=3>"
		)
	endif()
	if((CMAKE_CXX_COMPILER_ID MATCHES [[Clang]]) OR EMSCRIPTEN)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(okiidoku::compiler_warnings INTERFACE
			-fno-nonansi-builtins # Disable built-in declarations of functions that are not mandated by ANSI/ISO C. These include ffs, alloca, _exit, index, bzero, conjf, and other related functions.
			# -fno-implicit-templates # https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html#index-fno-implicit-templates
		)
	endif()
endif()


# warnings:
# (for diagnostics that don't change the emitted object files)
# note that usage of @file is not introspected by ccache or ninja's dirty checks,
#  so avoid it for anything other than diagnostics-related flags.
if(MSVC)
	target_compile_options(okiidoku::compiler_warnings INTERFACE
		/W4 # highest warnings level
	)
else()
	set(flag_file_dir "${okiidoku_SOURCE_DIR}/cmake/okiidoku/compile_opts")
	# set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${flags_file}")
	target_compile_options(okiidoku::compiler_warnings INTERFACE
		-Wfatal-errors # stop compilation on first error. I found it hard to read multiple.
		"@${flag_file_dir}/warnings.gnu.txt"
		# -Warith-conversion
		# -Wframe-larger-than=byte-size -Wstack-usage=byte-size
	)
	if((CMAKE_CXX_COMPILER_ID MATCHES [[Clang]]) OR EMSCRIPTEN)
		target_compile_options(okiidoku::compiler_warnings INTERFACE
			-Wimplicit-fallthrough
			-Wno-unknown-attributes # (I'd like to use this, but it's just too annoying, flagging benign stuff).
			-Wunsafe-buffer-usage
		)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(okiidoku::compiler_warnings INTERFACE
			"@${flag_file_dir}/warnings.gcc.txt"

			# -Wunsafe-loop-optimizations # only meaningful with -funsafe-loop-optimizations

			# interesting but probably too overboard:
			# -Wpadded
		)
	endif()
endif()
