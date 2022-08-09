# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)
include(CheckCXXSourceCompiles)

if(MSVC)
	add_compile_options(/EHsc)
	# https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model#standard-c-exception-handling
	# must be consistent for whole program(s). do not hijack user's choice.
	# This project doesn't throw exceptions, so any model should be okay.
	# TODO wat https://gitlab.kitware.com/cmake/cmake/-/issues/20610
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	add_compile_options(
		-fno-semantic-interposition # disable ELF external overrides for more optimization (on by default for LLVM)
	)
endif()

if(NOT MSVC)
	add_compile_options(
		-march=native # enable optimizing for the compiling machine's instruction set architecture.
	)
endif()


# Note: wrapping with functions to scope `CMAKE_REQUIRED_FLAGS`
# cspell:dictionaries cpp-refined
function(detect_target_isa_support)
	if(NOT MSVC)
		set(CMAKE_REQUIRED_FLAGS -march=native)
	endif()
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
endfunction()
detect_target_isa_support()