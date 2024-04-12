# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:ignoreRegExp -[W][a-z-]+\b
include_guard(DIRECTORY)

add_library(okiidoku_compile_options_public INTERFACE)
add_library(okiidoku_compiler_warnings INTERFACE IMPORTED)
# Note: "IMPORTED" used to prevent auto installation

function(okiidoku_add_compiler_options target)
	target_link_libraries(${target}
		PUBLIC okiidoku_compile_options_public
	)
	if(OKIIDOKU_BUILD_WITH_SUGGESTED_WARNINGS)
		target_link_libraries(${target}
			PRIVATE okiidoku_compiler_warnings
		)
	endif()
	# get_target_property(sources ${target} SOURCES)
	# set_property(SOURCE ${sources}
	# 	# DIRECTORY "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}"
	# 	TARGET_DIRECTORY ${target}
	# 	APPEND PROPERTY OBJECT_DEPENDS "${PROJECT_SOURCE_DIR}/cmake/okiidoku/flags"
	# )
	# # TODO.asap delete commented out code
	# message("${sources}")
	# foreach(source ${sources})
	# 	get_source_file_property(deps "${source}" DIRECTORY "${PROJECT_SOURCE_DIR}" OBJECT_DEPENDS)
	# 	message("dep: ${source}: ${deps}")
	# endforeach()
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
		"$<${v12}:-Wbidi-chars=any>" # warn on any usage of bidi text
		-Wnormalized # warn on identifiers that look the same but are not the same
		-Wno-unknown-pragmas
		"$<${v12}:-Wno-attributes=clang::>"
	)
endblock()
endif()


# diagnostics formatting:
# TODO hm. these make more sense to be user-controlled globally-applied flags. maybe should be moved to CMakePresets.json
if(PROJECT_IS_TOP_LEVEL)
	if(MSVC)
		target_compile_options(okiidoku_compiler_warnings INTERFACE
			/diagnostics:caret
		)
	else()
		target_compile_options(okiidoku_compiler_warnings INTERFACE
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
if(MSVC)
	target_compile_options(okiidoku_compiler_warnings INTERFACE
	)
else()
	target_compile_options(okiidoku_compiler_warnings INTERFACE
		-Werror=date-time
		# "-ffile-prefix-map=${CMAKE_SOURCE_DIR}=." # TODO.low try this out and see what benefits there are
	)
endif()


# warnings:
if(MSVC)
	target_compile_options(okiidoku_compiler_warnings INTERFACE
		/W4 # highest warnings level
	)
else()
	# set(flags_file "${PROJECT_SOURCE_DIR}/cmake/okiidoku/compile_opts/warnings.gcc.txt")
	# set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${flags_file}")
	target_compile_options(okiidoku_compiler_warnings INTERFACE
		-Wfatal-errors # stop compilation on first error. I found it hard to read multiple.
		# "@${flags_file}"
		-Wall -Wextra -Wpedantic -pedantic-errors
		-Wold-style-cast
		-Wvla # maybe put this in the project-root cmake file
		-Wconversion -Wsign-conversion #-Warith-conversion
		-Wdouble-promotion
		# -Wframe-larger-than=byte-size -Wstack-usage=byte-size

		-Wdisabled-optimization # not an error. notification that function is too big. gcc gave up on optimizing.

		-Wformat=2
		-Wcast-qual -Wcast-align
		-Wshadow -Woverloaded-virtual
		-Wnon-virtual-dtor
		# -Wnoexcept
		-Wenum-compare -Wenum-conversion
		-Wmissing-declarations -Wunused-macros
		-Wsuggest-override
		-Wmismatched-tags -Wextra-semi
		-Wundef # warn on undefined identifier used in `#if`
	)
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR EMSCRIPTEN)
		target_compile_options(okiidoku_compiler_warnings INTERFACE
			-Wimplicit-fallthrough
		)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(okiidoku_compiler_warnings INTERFACE
			-fno-nonansi-builtins # Disable built-in declarations of functions that are not mandated by ANSI/ISO C. These include ffs, alloca, _exit, index, bzero, conjf, and other related functions.
			-Wtrampolines
			-Wimplicit-fallthrough=5

			-Wlogical-op
			-Wduplicated-cond
			-Walloc-zero
			-Wuseless-cast

			-Wredundant-tags

			-Wsuggest-final-types
			-Wsuggest-final-methods
			-Wsuggest-attribute=pure
			-Wsuggest-attribute=const
			-Wsuggest-attribute=noreturn
			# TODO.try
			# -fno-implicit-templates or -frepo # https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Template-Instantiation.html#Template-Instantiation

			# -Wunsafe-loop-optimizations # only meaningful with -funsafe-loop-optimizations

			# interesting but probably too overboard:
			# -Wpadded
			# -Wsign-promo # actually warns on overload selection behaviour mandated by the standard :O
		)
	endif()
endif()
