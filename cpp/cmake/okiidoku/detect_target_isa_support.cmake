# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)

include(CheckCXXSourceCompiles)
# Note: wrapping with functions to scope `CMAKE_REQUIRED_FLAGS`

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