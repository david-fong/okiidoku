# https://cmake.org/cmake/help/latest/module/FetchContent.html
include(FetchContent)

FetchContent_Declare(range-v3 # https://github.com/ericniebler/range-v3
	GIT_REPOSITORY git@github.com:ericniebler/range-v3.git
	GIT_TAG master # 0.11.0 is currently the latest official release, but it pretty old (2020). using master as a crutch to get newer fixes
	GIT_SHALLOW TRUE
	GIT_SUBMODULES "" # none. don't get the docs submodule
	GIT_PROGRESS ON
	# SOURCE_DIR "${okiidoku_SOURCE_DIR}/external/range-v3"
	# https://discourse.cmake.org/t/share-fetchcontent-between-projects/4537/2
)


# Note: make sure to do all `Declare`s before any `MakeAvailable`s.

FetchContent_MakeAvailable(range-v3)

# read about why this is done in `writing/journal/cpp-dependencies.md`
get_target_property(RANGE_V3_IID range-v3::range-v3 INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(range-v3 PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${RANGE_V3_IID}")
unset(RANGE_V3_IID)