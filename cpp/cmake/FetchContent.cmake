include_guard(DIRECTORY)

# https://cmake.org/cmake/help/latest/module/FetchContent.html
include(FetchContent)

FetchContent_Declare(
	range-v3 # https://github.com/ericniebler/range-v3
	GIT_REPOSITORY git@github.com:ericniebler/range-v3.git
	GIT_TAG "master" # 0.11.0 is currently the latest official release, but it's very old (2020). using master as a crutch to get newer fixes
	GIT_SHALLOW TRUE
	GIT_SUBMODULES "" # none. don't get the docs submodule
	GIT_PROGRESS ON
	# placement in `external/` to allow deleting `build/` without deleting the range-v3 clone.
	PREFIX       "${PROJECT_SOURCE_DIR}/external/range-v3/cmake"
	SOURCE_DIR   "${PROJECT_SOURCE_DIR}/external/range-v3/repo"
	BINARY_DIR   "${PROJECT_SOURCE_DIR}/external/range-v3/build"
	SUBBUILD_DIR "${PROJECT_SOURCE_DIR}/external/range-v3/subbuild"
	# https://github.com/ericniebler/range-v3/issues/1689
	# https://discourse.cmake.org/t/share-fetchcontent-between-projects/4537/2
	# CMake options I might care about:
	# - RANGES_MODULES, RANGES_POLLY, RANGES_DEEP_STL_INTEGRATION
)
set(FETCHCONTENT_UPDATES_DISCONNECTED_RANGE-V3 TRUE)


FetchContent_Declare(
	pybind11 # https://pybind11.readthedocs.io/en/stable/index.html
	GIT_REPOSITORY git@github.com:pybind/pybind11.git
	GIT_TAG "v2.9.2" # https://pybind11.readthedocs.io/en/stable/changelog.html
	GIT_SHALLOW TRUE
	GIT_SUBMODULES ""
	GIT_PROGRESS ON
	# placement in `external/` to allow deleting `build/` without deleting the range-v3 clone.
	PREFIX       "${PROJECT_SOURCE_DIR}/external/pybind11/cmake"
	SOURCE_DIR   "${PROJECT_SOURCE_DIR}/external/pybind11/repo"
	BINARY_DIR   "${PROJECT_SOURCE_DIR}/external/pybind11/build"
	SUBBUILD_DIR "${PROJECT_SOURCE_DIR}/external/pybind11/subbuild"
)
set(FETCHCONTENT_UPDATES_DISCONNECTED_PYBIND11 TRUE)


# yes, I know _all of this_ is not recommended practice, but it seems
# suitable for this project's usage of range-v3 at the current time.
# its root CMakeLists.txt seems to do many things that are only necessary
# when developing (and not when consuming) it.
FetchContent_GetProperties(range-v3)
if(NOT range-v3_POPULATED)
	# Note: for some reason populated always seems to be false?
	# But it seems ExternalProject_Add will still detect no-need for re-clone?
	FetchContent_Populate(range-v3)
	add_library(range-v3 INTERFACE IMPORTED)
	add_library(range-v3::range-v3 ALIAS range-v3)
	target_include_directories(range-v3 SYSTEM INTERFACE "${range-v3_SOURCE_DIR}/include")
endif()