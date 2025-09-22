# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)
if(NOT okiidoku_IS_TOP_LEVEL)
	return()
endif()

# see also tools/cmake_top_project_include.cmake
# do not wrap these with `block()`

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
	if(GCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "15")
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
	#add_compile_options("-fipa-reorder-for-locality") # TODO.wait GCC v15
	# https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#index-fipa-reorder-for-locality
	# If using this option it is recommended to also use profile feedback
	# otherwise https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#index-freorder-functions default for -O2, -O3, -Os.
endif()


# ccache
if(NOT DEFINED CMAKE_CXX_COMPILER_LAUNCHER)
	find_program(CCACHE_EXE NAMES ccache DOC "Path to ccache executable")
	if(NOT "${CCACHE_EXE}" STREQUAL "CCACHE_EXE-NOTFOUND")
		set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXE}"
			# "--config-path" "${okiidoku_SOURCE_DIR}/tools/ccache.conf"
			# man ccache @"COMPILING IN DIFFERENT DIRECTORIES":
			# "base_dir=${okiidoku_SOURCE_DIR}/.." "hash_dir=false"
			# "run_second_cpp=false"
		)
		# add_compile_options( # see run_second_cpp docs
		# 	"$<$<CXX_COMPILER_ID:GNU>:-fdirectives-only>"
		# 	"$<$<CXX_COMPILER_ID:Clang>:-frewrite-includes>"
		# )
		set(CMAKE_C_COMPILER_LAUNCHER "${CMAKE_CXX_COMPILER_LAUNCHER}")
	endif()
endif()


# ninjatracing
# TODO.wait https://www.kitware.com/new-cmake-instrumentation-feature-provides-detailed-timing-of-builds/  https://www.kitware.com/tag/cmake/
if(CMAKE_GENERATOR MATCHES [[^Ninja]])
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
