include_guard(DIRECTORY)

set(CPM_DOWNLOAD_VERSION 0.35.1) # https://github.com/cpm-cmake/CPM.cmake/tags
include(okiidoku/dependencies.get_cpm)


CPMAddPackage(
	NAME range-v3
	GIT_TAG 0.12.0 # https://github.com/ericniebler/range-v3/releases
	GITHUB_REPOSITORY "ericniebler/range-v3"
	DOWNLOAD_ONLY TRUE
)
if(range-v3_ADDED)
	add_library(range-v3 INTERFACE IMPORTED)
	add_library(range-v3::range-v3 ALIAS range-v3)
	target_include_directories(range-v3 SYSTEM INTERFACE "${range-v3_SOURCE_DIR}/include")
	# https://github.com/ericniebler/range-v3/issues/1689
	# original CMake options I might care about:
	#  RANGES_MODULES, RANGES_POLLY, RANGES_DEEP_STL_INTEGRATION
endif()


if(okiidoku_IS_TOP_LEVEL)
	CPMAddPackage(
		NAME catch2
		VERSION 3.0.1
		GITHUB_REPOSITORY "catchorg/Catch2"
	)
	include("${catch2_SOURCE_DIR}/extras/Catch.cmake")
endif()