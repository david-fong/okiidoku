add_library(okiidoku_gpu STATIC
	src/okiidoku_gpu/gen.cpp
)

# ubuntu installation:
# apt install ocl-icd-opencl-dev
# do I need this? https://www.intel.com/content/www/us/en/developer/tools/opencl-sdk/choose-download.html
find_package(OpenCL REQUIRED)

# target_include_directories(okiidoku_gpu ${OPENCL_INCLUDE_DIR}) # TODO.try is this needed?
target_include_directories(okiidoku_gpu
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	# $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_sources(okiidoku_gpu PUBLIC FILE_SET HEADERS BASE_DIRS include)
target_link_libraries(okiidoku_gpu PUBLIC OpenCL::OpenCL)