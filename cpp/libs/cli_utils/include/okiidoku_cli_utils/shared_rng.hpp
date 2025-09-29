// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__RNG
#define HPP_OKIIDOKU__RNG

//#include <pcg_random.hpp>

#include <random>
#include <mutex>
#include <cstdint>

namespace okiidoku::util {

	class SharedRng {
	public:
		// Internal Notes:
		// - consider not using this for any hot-loop operations.
		// - this is not thread-safe. guard with mutex where necessary.
		using rng_t = std::mt19937_64;

		explicit SharedRng(rng_t::result_type seed) noexcept: rng{seed} {}

		[[nodiscard]] std::uint_fast32_t operator()() noexcept {
			std::scoped_lock scoped_lock {mutex};
			#ifndef __clang__
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wuseless-cast" // not useless on MSVC
			#endif
			return static_cast<std::uint_fast32_t>(rng() - decltype(rng)::min());
			#ifndef __clang__
			#pragma GCC diagnostic pop
			#endif
		}
	private:
		rng_t rng;
		mutable std::mutex mutex {};
	};
}
#endif
