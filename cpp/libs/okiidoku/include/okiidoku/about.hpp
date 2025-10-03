// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_ABOUT
#define HPP_OKIIDOKU_ABOUT

#include <string_view>
#include <array>
#include <cstdint>

namespace okiidoku::about {

	struct SemanticVersion {
		std::uint32_t major;
		std::uint32_t minor;
		std::uint32_t patch;
		std::uint32_t tweak;
	};
	OKIIDOKU_EXPORT extern const SemanticVersion semver;

	struct GitInfo {
		std::string_view remotes;
		std::string_view branch;
		std::string_view commit;
	};
	OKIIDOKU_EXPORT extern const GitInfo git_info;
}

#endif