#include <CL/opencl.hpp>
#include <string>
#include "solvent_lib_gpu/gen.hpp"

int main() {
	cl::Context context(CL_DEVICE_TYPE_CPU);
	cl::CommandQueue queue(context);

	std::string kernel_source = ""; // TODO load from file?
	cl::Program program(context, kernel_source, /*compile:*/true);
	// inputs for each generator: order, val_try_order, max_backtrack_count, canonicalize
	// outputs for each generator: status, grid, 
	// cl::Buffer d_val_try_order(context, );
}