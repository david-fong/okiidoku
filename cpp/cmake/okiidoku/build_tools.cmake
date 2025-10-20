# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)

# see also tools/cmake_top_project_include.cmake
# do not wrap these with `block()`


if(EMSCRIPTEN)
	add_compile_options(-sSTRICT=1)
	add_link_options(-sSTRICT=1)
endif()
if(EMSCRIPTEN AND okiidoku_IS_TOP_LEVEL)
	# https://emscripten.org/docs/tools_reference/emcc.html#emcc-gsource-map
	add_link_options("$<${debug_configs}:-gsource-map>")
endif()


# `-D _GLIBCXX_DEBUG` for libstdc++
include(CheckCXXSymbolExists)
if(NOT DEFINED CACHE{_OKIIDOKU_IS_LIBSTDCXX})
	check_cxx_symbol_exists("__GLIBCXX__" "version" _OKIIDOKU_IS_LIBSTDCXX)
	# (libc++ from LLVM would check for `_LIBCPP_VERSION`)
endif()
if("$CACHE{_OKIIDOKU_IS_LIBSTDCXX}")
	add_compile_definitions(
		# https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_macros.html
		"$<${debug_configs}:_GLIBCXX_DEBUG>" # implies _GLIBCXX_ASSERTIONS
		"$<${debug_configs}:_GLIBCXX_DEBUG_PEDANTIC>" # usage of libstdc++ extensions are errors
	)
endif()


# related to reproducible builds / deterministic compilation:
if(NOT MSVC)
	add_compile_options(
		# "-Werror=date-time" # error to use __TIME__, __DATE__ or __TIMESTAMP__
		"-fno-record-gcc-switches" "-gno-record-gcc-switches"
		"-ffile-prefix-map=${okiidoku_SOURCE_DIR}=/okiidoku"
		#  see also gdb: set substitute-path, lldb: target.source-map, vscode: "sourceFileMap"
		# TODO: why am I still getting absolute paths in debug builds for GCC?
	)
	if(EMSCRIPTEN AND okiidoku_IS_TOP_LEVEL)
		# https://emscripten.org/docs/tools_reference/settings_reference.html#source-map-prefixes
		# https://github.com/emscripten-core/emscripten/blob/main/ChangeLog.md#406---032625
		add_link_options("$<${debug_configs}:-sSOURCE_MAP_PREFIXES=${okiidoku_SOURCE_DIR}=/okiidoku>") # TODO.low this doesn't seem to be working. it's using relative paths...
	endif()
	# see also :/cpp/cmake/okiidoku/source_file_random_seed.cmake.
endif()
if((CMAKE_CXX_COMPILER_ID MATCHES [[Clang]]) OR EMSCRIPTEN)
	add_compile_options("-fno-record-command-line")
endif()


if(UNIX AND NOT EMSCRIPTEN)
add_link_options(
	LINKER:--package-metadata=JSON # https://systemd.io/PACKAGE_METADATA_FOR_EXECUTABLE_FILES/
	LINKER:--build-id
	LINKER:--compress-debug-sections=zstd
)
endif()


# link-time optimization things
include(CheckIPOSupported)
check_ipo_supported(RESULT okiidoku_is_ipo_supported)
if((NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION) AND "${okiidoku_is_ipo_supported}")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
	if(GCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "15")
		add_compile_options("-flto-incremental=${OKIIDOKU_LTO_CACHE_DIR}")
	endif()
endif()

# linker section garbage collection
if(NOT "${CMAKE_CXX_COMPILER_LINKER_FRONTEND_VARIANT}" STREQUAL "MSVC")
	if(NOT "${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")
		add_compile_options("-ffunction-sections" "-fdata-sections")
	endif()
	add_link_options("LINKER:--gc-sections")
endif()
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	# add_compile_options("-fipa-reorder-for-locality") # doesn't seem to make a difference :.
	# add_link_options("-fipa-reorder-for-locality") # not sure if this is needed
	# https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#index-fipa-reorder-for-locality
	# If using this option it is recommended to also use profile feedback
	# otherwise https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#index-freorder-functions default for -O2, -O3, -Os.
endif()


# ccache
if(NOT DEFINED CMAKE_CXX_COMPILER_LAUNCHER)
find_program(CCACHE_EXE NAMES ccache DOC "Path to ccache executable")
if(NOT "${CCACHE_EXE}" STREQUAL "CCACHE_EXE-NOTFOUND")
	message("using ccache with namespace `okiidoku.${CMAKE_CXX_COMPILER_ID}.$<CONFIG>`") # (for `ccache --evict-namespace`)
	set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXE}"
		"namespace = okiidoku.${CMAKE_CXX_COMPILER_ID}.$<CONFIG>"
		"ignore_options = /w* /W* -W* -ftabstop=*"
		# man ccache @"COMPILING IN DIFFERENT DIRECTORIES":
		"base_dir = ${okiidoku_SOURCE_DIR}/.."
		"absolute_paths_in_stderr = true"
		"hash_dir = false"
		# "run_second_cpp=false"
	)
	if(NOT OKIIDOKU_BUILD_REPRODUCIBLE)
		list(APPEND CMAKE_CXX_COMPILER_LAUNCHER
			"sloppiness = locale,random_seed,time_macros"
		)
	endif()
	# add_compile_options( # see run_second_cpp docs
	# 	"$<$<CXX_COMPILER_ID:GNU>:-fdirectives-only>"
	# 	"$<$<CXX_COMPILER_ID:Clang>:-frewrite-includes>"
	# )
	set(CMAKE_C_COMPILER_LAUNCHER "${CMAKE_CXX_COMPILER_LAUNCHER}")
endif()
endif()


# ninjatracing
# TODO.wait https://www.kitware.com/new-cmake-instrumentation-feature-provides-detailed-timing-of-builds/  https://www.kitware.com/tag/cmake/
if(okiidoku_IS_TOP_LEVEL AND (CMAKE_GENERATOR MATCHES [[^Ninja]]))
install(CODE #[[build perf data]] "
file(MAKE_DIRECTORY \"${OKIIDOKU_DATA_OUTPUT_DIRECTORY}\")
execute_process(
	# TODO.optional: finer-grained for clang https://crascit.com/2022/06/24/build-performance-insights/#:~:text=ninja_log%20%3E%20cmake_build_trace.json-,Clang%20Time%20Tracing,-If%20you%20are
	COMMAND \"${okiidoku_SOURCE_DIR}/tools/ninjatracing.py\" \"${CMAKE_BINARY_DIR}/.ninja_log\"
	OUTPUT_FILE \"${OKIIDOKU_DATA_OUTPUT_DIRECTORY}/trace.json\"
	ECHO_ERROR_VARIABLE
)
message(\"generated ninja trace at <file://${OKIIDOKU_DATA_OUTPUT_DIRECTORY}/trace.json>\")
message(\"view at https://ui.perfetto.dev\")
")
endif()
