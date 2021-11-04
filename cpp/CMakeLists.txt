cmake_minimum_required(VERSION 3.16)
project(solvent)

# links to resources because I'm learning CMake
# https://cmake.org/cmake/help/latest/guide/tutorial/index.html
# https://cliutils.gitlab.io/modern-cmake/
# https://github.com/lefticus/cpp_weekly_game_project

set(CMAKE_CXX_STANDARD 20 CACHE)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)
if(MSVC)
	add_compile_options(/W4 /WX)
else()
	add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wold-style-cast -Wimplicit-fallthrough=5 -march=native)
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

add_subdirectory(src/solvent_lib)
add_subdirectory(src/solvent_cli)
add_subdirectory(src/solvent_web)


