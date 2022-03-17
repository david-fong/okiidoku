#ifndef HPP_SOLVENT__RNG
#define HPP_SOLVENT__RNG

#include "solvent_export.h"

#include <random>
#include <mutex>

namespace solvent {

	/* Internal Notes:
	- consider not using this for any hot-loop operations.
	- this is not thread-safe. guard with mutex where necessary.
	*/
	extern std::mt19937_64 shared_mt_rng_;
	extern std::mutex shared_mt_rng_mutex_;

	// Optional; rng is automatically seeded using `std::random_device` on startup.
	SOLVENT_EXPORT void seed_rng(const std::uint_fast64_t seed) noexcept;
}
#endif