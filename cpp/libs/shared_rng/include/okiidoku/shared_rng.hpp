#ifndef HPP_OKIIDOKU__RNG
#define HPP_OKIIDOKU__RNG

#include <okiidoku_export.h>

#include <random>
#include <mutex>

/* #define M_OKIIDOKU_DEFINE_MT19937_64 \
template class std::mersenne_twister_engine<std::uint_fast64_t, 64, 312, 156, 31, \
	0xb5026f5aa96619e9, 29, \
	0x5555555555555555, 17, \
	0x71d67fffeda60000, 37, \
	0xfff7eee000000000, 43, 6364136223846793005 \
>; */

// explicit template declaration of std::mt19937_64.
// done to prevent inclusion of the implementation in each per-order dynamic library.
// requires users of the code to provide the
// extern M_OKIIDOKU_DEFINE_MT19937_64

// #ifndef M_OKIIDOKU_IS_BUILD_LIBRARY
// M_OKIIDOKU_DEFINE_MT19937_64
// #endif

namespace okiidoku {

	struct OKIIDOKU_EXPORT SharedRng final {
		// Internal Notes:
		// - consider not using this for any hot-loop operations.
		// - this is not thread-safe. guard with mutex where necessary.
		std::mt19937_64 shared_mt_rng_;
		std::mutex shared_mt_rng_mutex_;
	};
}
#endif