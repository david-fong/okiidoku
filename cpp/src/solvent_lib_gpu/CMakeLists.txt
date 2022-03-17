add_library(solvent_gpu STATIC
	gen.cpp
)

# ubuntu installation:
# apt install ocl-icd-opencl-dev
# do I need this? https://www.intel.com/content/www/us/en/developer/tools/opencl-sdk/choose-download.html
find_package(OpenCL REQUIRED)

# target_include_directories(solvent_gpu ${OPENCL_INCLUDE_DIR}) # TODO.try is this needed?
target_include_directories(solvent_gpu PRIVATE ..)
target_include_directories(solvent_gpu PUBLIC ../../include)
target_link_libraries(solvent_gpu PUBLIC OpenCL::OpenCL)