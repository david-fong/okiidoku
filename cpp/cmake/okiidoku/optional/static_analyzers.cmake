# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:dictionaries cpp-refined
include_guard(DIRECTORY)

if(MSVC)
	target_compile_options(okiidoku_compiler_warnings INTERFACE "/analyze")
	# https://docs.microsoft.com/en-us/cpp/build/reference/analyze-code-analysis
else()
	# target_compile_options(okiidoku_compiler_warnings INTERFACE "-fanalyzer")
	# https://gcc.gnu.org/onlinedocs/gcc/Static-Analyzer-Options.html
endif()


find_program(
	CLANG_TIDY_EXE
	NAMES "clang-tidy"
	DOC "Path to clang-tidy executable"
)
if(NOT "${CLANG_TIDY_EXE}" STREQUAL "CLANG_TIDY_EXE-NOTFOUND")
	set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
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