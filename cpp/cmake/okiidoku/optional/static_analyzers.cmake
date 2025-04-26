# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:dictionaries cpp-refined
include_guard(DIRECTORY)
if(NOT OKIIDOKU_BUILD_WITH_STATIC_ANALYZERS)
	return()
endif()
block()

if(MSVC)
	target_compile_options(okiidoku_compiler_warnings INTERFACE "/analyze")
	#^ https://docs.microsoft.com/en-us/cpp/build/reference/analyze-code-analysis
else()
	target_compile_options(okiidoku_compiler_warnings INTERFACE "$<$<COMPILE_LANGUAGE:C>:-fanalyzer>")
	#^ https://gcc.gnu.org/onlinedocs/gcc/Static-Analyzer-Options.html
	#  it's intended for C code and unlikely to be useful for C++ (see https://gcc.gnu.org/gcc-13/changes.html)
	target_compile_definitions(okiidoku_compiler_warnings INTERFACE
		"$<${debug_configs}:_GLIBCXX_CONCEPT_CHECKS>"
	)
endif()


find_program(
	CLANG_TIDY_EXE
	NAMES "clang-tidy"
	DOC "Path to clang-tidy executable"
)
if(NOT "${CLANG_TIDY_EXE}" STREQUAL "CLANG_TIDY_EXE-NOTFOUND")
	set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
	# if(CMAKE_COLOR_DIAGNOSTICS) # it already detects if terminal supports colours
	# 	list(APPEND CMAKE_CXX_CLANG_TIDY "--use-color")
	# endif()
	# TODO.mid sort out the warnings
endif()


find_program(
	CPPCHECK_EXE
	NAMES "cppcheck"
	DOC "Path to cppcheck executable"
)
if(NOT "${CPPCHECK_EXE}" STREQUAL "CPPCHECK_EXE-NOTFOUND")
	set(CMAKE_CXX_CPPCHECK "${CPPCHECK_EXE}" --inline-suppr)
	# enable suppressing inline with comments like: "cppcheck-suppress [aaaa,bbbb]"
	# It's choking on the tweak header's `#if __has_include(...)`.
	# Other people also mention this issue on sourceforge.
endif()

endblock()