cmake_minimum_required(VERSION 3.16)
project(solvent)

# links to resources because I'm learning CMake
# https://cmake.org/cmake/help/latest/guide/tutorial/index.html
# https://cliutils.gitlab.io/modern-cmake/
# https://github.com/lefticus/cpp_weekly_game_project
# https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs

# cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
# cmake --build build/release

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
		-march=native
	)
endif()

set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# https://cliutils.gitlab.io/modern-cmake/chapters/testing.html
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	include(CTest)
endif()


add_subdirectory(src/solvent_util)
add_subdirectory(src/solvent)
# add_subdirectory(src/solvent_gpu)

add_subdirectory(src/solvent_cli)
# add_subdirectory(src/solvent_web)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
	add_subdirectory(tests)
endif()