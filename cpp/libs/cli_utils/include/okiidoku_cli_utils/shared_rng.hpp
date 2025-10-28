// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_RNG
#define HPP_OKIIDOKU_RNG

#include <pcg_random.hpp>

namespace okiidoku::util {
	using Prng = pcg64_fast;
}
#endif