include_guard(DIRECTORY)

set(CPM_DOWNLOAD_VERSION 0.35.1) # https://github.com/cpm-cmake/CPM.cmake/tags
# https://github.com/cpm-cmake/CPM.cmake/blob/master/cmake/get_cpm.cmake

if(CPM_SOURCE_CACHE)
	# Expand relative path. This is important if the provided path contains a tilde (~)
	get_filename_component(CPM_SOURCE_CACHE ${CPM_SOURCE_CACHE} ABSOLUTE)
	set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
	set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
	set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
	message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
	file(DOWNLOAD
		https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
		${CPM_DOWNLOAD_LOCATION}
	)
endif()

include(${CPM_DOWNLOAD_LOCATION})


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
	# original CMake options I might care about:
	#  RANGES_MODULES, RANGES_POLLY, RANGES_DEEP_STL_INTEGRATION
endif()