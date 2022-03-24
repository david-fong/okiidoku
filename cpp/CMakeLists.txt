cmake_minimum_required(VERSION 3.16)
project(solvent
	VERSION 0.0.1
	DESCRIPTION "sudoku solution generator and canonicalizer"
	HOMEPAGE_URL "https://github.com/david-fong/solvent"
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)
if(MSVC)
	add_compile_options(/W4 /WX)
else()
	add_compile_options(
		-Wall -Wextra -Wpedantic -Werror
		-Wold-style-cast -Wcast-qual -Wcast-align -Wuseless-cast
		-Wformat=2
		-Wconversion # -Wsign-conversion
		-Wimplicit-fallthrough=5
		-Wshadow -Woverloaded-virtual
		-march=native # generated code meant only to run on compiling machine. for the speedup.
	)
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
include(GNUInstallDirs)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")

# https://cliutils.gitlab.io/modern-cmake/chapters/testing.html
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	include(CTest)
endif()


add_subdirectory(src/solvent_util)
add_subdirectory(src/solvent)
# add_subdirectory(src/solvent_gpu)

add_subdirectory(src/solvent_cli)
# add_subdirectory(src/solvent_web)

add_subdirectory(tools)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
	add_subdirectory(tests)
endif()


# try using FILE_SETs when more people have cmake 3.23
install(DIRECTORY include/solvent DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS solvent solvent_cli EXPORT solventTargets)
install(EXPORT solventTargets
	FILE solventTargets.cmake
	NAMESPACE solvent::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/solvent
)


# Packaging support
# https://cmake.org/cmake/help/latest/module/CPack.html
set(CPACK_PACKAGE_VENDOR "david-fong")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
set(CPACK_SOURCE_GENERATOR "TGZ")
include(CPack)