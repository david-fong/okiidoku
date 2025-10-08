# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:dictionaries cpp-refined
include_guard(DIRECTORY)

block()
set(output "${CMAKE_CURRENT_BINARY_DIR}/src/okiidoku/about.semver.cpp")
file(CONFIGURE OUTPUT ${output} CONTENT
"// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/about.hpp>
namespace okiidoku::about {
	constinit const SemanticVersion semver {
		.major = ${okiidoku_VERSION_MAJOR},
		.minor = ${okiidoku_VERSION_MINOR},
		.patch = ${okiidoku_VERSION_PATCH},
		// .tweak = ${okiidoku_VERSION_TWEAK},
		.tweak = 0,
	};
}")
target_sources(okiidoku PRIVATE "${output}")
endblock()

install(
	SCRIPT "${CMAKE_CURRENT_LIST_DIR}/generate_meta_info.git.cmake"
	COMPONENT okiidoku_about
	OPTIONAL
)