cmake_minimum_required(VERSION 3.23)
project(okiidoku
	VERSION 0.0.1
	DESCRIPTION "sudoku solution generator, canonicalizer, and db"
	HOMEPAGE_URL "https://github.com/david-fong/okiidoku"
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)
if(MSVC)
	add_compile_options(
		/utf-8        # /source-charset:utf-8 (for preprocessor), and /execution-charset:utf8 (for compiler)
		/permissive-  # https://discourse.cmake.org/t/cxx-extensions-and-permissive/1994
		/EHsc         # https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model#standard-c-exception-handling
		/volatile:iso # https://docs.microsoft.com/en-us/cpp/build/reference/volatile-volatile-keyword-interpretation#remarks
		/W4 #/WX      # highest warning level #and treat warnings as errors.
		/wd5030       # disable the "unrecognized attribute" warning. unfortunately needed or else MSVC warns for gnu-prefixed attributes.
	)
else()
	add_compile_options(
		-Wall -Wextra -Wpedantic -Wfatal-errors #-Werror
		-Wold-style-cast -Wcast-qual -Wcast-align 
		-Wformat=2
		-Wconversion # -Wsign-conversion
		-Wshadow -Woverloaded-virtual

		-march=native # generated code meant only to run on compiling machine. for the speedup.
		# TODO.wait see https://gcc.gnu.org/gcc-12/changes.html#uninitialized
	)
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		add_compile_options(
			-Wimplicit-fallthrough
		)
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		add_compile_options(
			-Wuseless-cast
			-Wimplicit-fallthrough=5
		)
	endif()
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)


if(MSVC)
	set(okiidoku_SHARED_LIBS FALSE)
elseif(PROJECT_IS_TOP_LEVEL)
	set(okiidoku_SHARED_LIBS TRUE)
endif()
if(DEFINED okiidoku_SHARED_LIBS)
	set(BUILD_SHARED_LIBS "${okiidoku_SHARED_LIBS}")
endif()
# set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
# if (NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET AND NOT DEFINED CMAKE_VISIBILITY_INLINES_HIDDEN)
	# use user-specified, or otherwise default to strict.
	set(CMAKE_CXX_VISIBILITY_PRESET hidden)
	set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)
# endif()
include(GNUInstallDirs)
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
if(NOT APPLE)
	set(CMAKE_INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
endif()

# https://cliutils.gitlab.io/modern-cmake/chapters/testing.html
if(PROJECT_IS_TOP_LEVEL)
	include(CTest)
endif()

find_program(
	CLANG_TIDY_EXE
	NAMES "clang-tidy"
	DOC "Path to clang-tidy executable"
)
if(NOT ${CLANG_TIDY_EXE} STREQUAL "CLANG_TIDY_EXE-NOTFOUND")
	# set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE})
	# TODO.mid sort out the warnings
endif()


# TODO.low https://docs.conan.io/en/latest/howtos/cmake_launch.html
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake) # Include Conan-generated file
conan_basic_setup(
	TARGETS # Introduce Conan-generated targets
	NO_OUTPUT_DIRS # don't put all target outputs under a bin/ directory of the build tree. if this works fine with MSVC, then maybe I'd be okay with allowing this.
)

add_subdirectory(libs/cli_utils)
add_subdirectory(libs/okiidoku)

add_subdirectory(apps/cli)
# add_subdirectory(apps/web)

add_subdirectory(tools)


install(TARGETS
	okiidoku okiidoku_cli
	EXPORT okiidokuTargets
	FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/okiidoku
	INCLUDES         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/okiidoku
)
if (BUILD_SHARED_LIBS)
	set(OKIIDOKU_LIB_TYPE_NAME shared)
else ()
	set(OKIIDOKU_LIB_TYPE_NAME static)
endif()
install(EXPORT okiidokuTargets
	FILE okiidoku-${OKIIDOKU_LIB_TYPE_NAME}-targets.cmake
	NAMESPACE okiidoku::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/okiidoku
)


# Packaging support
# https://cmake.org/cmake/help/latest/module/CPack.html
set(CPACK_PACKAGE_VENDOR "david-fong")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
set(CPACK_SOURCE_GENERATOR "TGZ")
# include(CPack) # disabled since currently using GCC's "-march=native".