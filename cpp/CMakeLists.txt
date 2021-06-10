cmake_minimum_required(VERSION 3.10)
project(solvent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)
if(MSVC)
	add_compile_options(/W4 /WX)
else()
	add_compile_options(-pedantic -Wall -Wextra -Wimplicit-fallthrough=5)
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

add_subdirectory(src/solvent_lib)
add_subdirectory(src/solvent_cli)
add_subdirectory(src/solvent_web)
