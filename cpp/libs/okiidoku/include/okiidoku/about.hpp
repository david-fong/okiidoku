// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_ABOUT
#define HPP_OKIIDOKU_ABOUT

#include <string_view>
#include <cstdint>

namespace okiidoku::about {

	struct [[gnu::designated_init]] SemanticVersion final {
		std::uint32_t major;
		std::uint32_t minor;
		std::uint32_t patch;
		std::uint32_t tweak;
	};
	OKIIDOKU_EXPORT extern constinit const SemanticVersion semver;
}

#endif