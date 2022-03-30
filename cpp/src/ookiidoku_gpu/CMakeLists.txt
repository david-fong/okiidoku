add_library(ookiidoku_gpu STATIC
	gen.cpp
)

# ubuntu installation:
# apt install ocl-icd-opencl-dev
# do I need this? https://www.intel.com/content/www/us/en/developer/tools/opencl-sdk/choose-download.html
find_package(OpenCL REQUIRED)

# target_include_directories(ookiidoku_gpu ${OPENCL_INCLUDE_DIR}) # TODO.try is this needed?
target_include_directories(ookiidoku_gpu
	PRIVATE "${PROJECT_SOURCE_DIR}/src"
	PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(ookiidoku_gpu PUBLIC OpenCL::OpenCL)