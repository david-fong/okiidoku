# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)
include(CheckCXXSourceCompiles)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(
		-fstrict-enums # assume that only declared values are possible
		-fno-semantic-interposition # disable ELF external overrides for more optimization (on by default for LLVM)
	)
endif()

if(NOT MSVC AND NOT DEFINED EMSCRIPTEN)
	add_compile_options(
		-march=native # enable optimizing for the compiling machine's instruction set architecture.
	)
endif()


# Note: wrapping with functions to scope changes to `CMAKE_CXX_FLAGS`
# cspell:dictionaries cpp-refined
block()
	get_directory_property(directory_compile_options COMPILE_OPTIONS)
	foreach(opt ${directory_compile_options})
		string(APPEND CMAKE_CXX_FLAGS " \"${opt}\"")
	endforeach()

	check_cxx_source_compiles("
		#include <immintrin.h>
		int main() {
			_pdep_u32(0, 0);
			_pdep_u64(0, 0);
			return 0;
		}
	" OKIIDOKU_TARGET_SUPPORTS_X86_BMI2)
	if(OKIIDOKU_TARGET_SUPPORTS_X86_BMI2)
		add_compile_definitions(OKIIDOKU_TARGET_SUPPORTS_X86_BMI2)
	endif()
endblock()