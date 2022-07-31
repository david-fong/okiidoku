# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:dictionaries cpp-refined
include_guard(DIRECTORY)

function(okiidoku_generate_version_semver)
	set(output "${CMAKE_CURRENT_BINARY_DIR}/src/okiidoku/about.semver.cpp")
	file(CONFIGURE OUTPUT ${output} CONTENT
"// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/about.hpp>
namespace okiidoku::about {
	const SemanticVersion semver {
		.major = ${okiidoku_VERSION_MAJOR},
		.minor = ${okiidoku_VERSION_MINOR},
		.patch = ${okiidoku_VERSION_PATCH},
		// .tweak = ${okiidoku_VERSION_TWEAK},
		.tweak = 0,
	};
}")
	target_sources(okiidoku PRIVATE "${output}")
endfunction()
okiidoku_generate_version_semver()


function(okiidoku_generate_version_git)
	set(output "${CMAKE_CURRENT_BINARY_DIR}/src/okiidoku/about.git.cpp")
	set(script "${okiidoku_SOURCE_DIR}/cmake/okiidoku/generate.git_info.configure_file.cmake")
	# Note: need to use add_custom_command instead of configure_file to execute at build-time.
	add_custom_command(
		COMMAND "${CMAKE_COMMAND}" "-DOUTPUT=${output}" -P "${script}"
		DEPENDS "${script}"
		OUTPUT  "${output}"
		VERBATIM
	)
	target_sources(okiidoku PRIVATE "${output}")
endfunction()
okiidoku_generate_version_git()