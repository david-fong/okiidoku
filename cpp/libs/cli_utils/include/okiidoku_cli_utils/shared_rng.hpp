#ifndef HPP_OKIIDOKU__RNG
#define HPP_OKIIDOKU__RNG

#include <random>
#include <mutex>
#include <cstdint>

namespace okiidoku {

	class SharedRng final {
	public:
		// Internal Notes:
		// - consider not using this for any hot-loop operations.
		// - this is not thread-safe. guard with mutex where necessary.
		std::mt19937_64 rng;
		mutable std::mutex mutex;

		[[nodiscard]] std::uint_fast32_t get_rng_seed() noexcept {
			std::lock_guard lock_guard {mutex};
			#ifndef __clang__
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wuseless-cast" // not useless on MSVC
			#endif
			return static_cast<std::uint_fast32_t>(rng() - decltype(rng)::min());
			#ifndef __clang__
			#pragma GCC diagnostic pop
			#endif
		}
	};
}
#endif