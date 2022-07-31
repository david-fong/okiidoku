// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU__ABOUT
#define HPP_OKIIDOKU__ABOUT

#include <okiidoku/detail/export.h>

#include <string_view>
#include <array>
#include <cstdint>

namespace okiidoku::about {

	struct SemanticVersion final {
		std::uint32_t major;
		std::uint32_t minor;
		std::uint32_t patch;
		std::uint32_t tweak;
	};
	OKIIDOKU_EXPORT extern const SemanticVersion semver;

	struct GitInfo final {
		std::string_view remotes;
		std::string_view branch;
		std::string_view commit;
	};
	OKIIDOKU_EXPORT extern const GitInfo git_info;
}

#endif