
#include "solvent/gen/backtracking.hpp"

#include <emscripten/bind.h>
#include <random>	// random_device,

//
int main(const int argc, char const *const argv[]) {

	// Scramble the random number generator (std::rand is no longer used):
	solvent::gen::Rng.seed(std::random_device()());

	return 0;
}

