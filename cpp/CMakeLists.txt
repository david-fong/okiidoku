cmake_minimum_required(VERSION 3.16)
project(okiidoku
	VERSION 0.0.1
	DESCRIPTION "sudoku solution generator and canonicalizer"
	HOMEPAGE_URL "https://github.com/david-fong/okiidoku"
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)
if(MSVC)
	add_compile_options(/W4 /WX
		/wd5030 # disable the "unrecognized attribute" warning. unfortunately needed or else MSVC warns for gnu-prefixed attributes.
	)
else()
	add_compile_options(
		-Wall -Wextra -Wpedantic -Werror
		-Wold-style-cast -Wcast-qual -Wcast-align -Wuseless-cast
		-Wformat=2
		-Wconversion # -Wsign-conversion
		-Wimplicit-fallthrough=5
		-Wshadow -Woverloaded-virtual
		-march=native # generated code meant only to run on compiling machine. for the speedup.
		# TODO.low see https://gcc.gnu.org/gcc-12/changes.html#uninitialized
	)
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)
include(GNUInstallDirs)
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")

# https://cliutils.gitlab.io/modern-cmake/chapters/testing.html
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	include(CTest)
endif()


# TODO.low https://docs.conan.io/en/latest/howtos/cmake_launch.html
# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake) # Include Conan-generated file
# conan_basic_setup(TARGETS) # Introduce Conan-generated targets

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
# TODO.mid install one of the generated export header somehow
install(EXPORT okiidokuTargets
	FILE okiidokuTargets.cmake
	# NAMESPACE okiidoku::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/okiidoku
)


# Packaging support
# https://cmake.org/cmake/help/latest/module/CPack.html
set(CPACK_PACKAGE_VENDOR "david-fong")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
set(CPACK_SOURCE_GENERATOR "TGZ")
include(CPack)