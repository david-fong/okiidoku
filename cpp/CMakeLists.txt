cmake_minimum_required(VERSION 3.10)
project(solvent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

add_subdirectory(src/solvent_lib)
add_subdirectory(src/solvent_cli)
add_subdirectory(src/solvent_web)
