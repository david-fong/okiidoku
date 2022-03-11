#include "solvent/rng.hpp"

namespace solvent {

	std::mt19937_64 shared_mt_rng_ {std::random_device{}()};
	std::mutex shared_mt_rng_mutex_ {};

	void seed_rng(const std::uint_fast64_t seed) noexcept {
		shared_mt_rng_.seed(seed);
	}
}