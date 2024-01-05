# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:dictionaries cpp-refined
include_guard(DIRECTORY)

block()
	set(output "${CMAKE_CURRENT_BINARY_DIR}/src/okiidoku/about.semver.cpp")
	file(CONFIGURE OUTPUT ${output} CONTENT
"// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <okiidoku/detail/pgo_use_check_needs_rebuild.hpp>
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
endblock()


block()
	find_package(Git QUIET)
	set(output "${CMAKE_CURRENT_BINARY_DIR}/src/okiidoku/about.git.cpp")
	set(script "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_meta_info.git.cmake")
	if(Git_FOUND)
		execute_process(
			COMMAND "${GIT_EXECUTABLE}" rev-parse --show-toplevel
			OUTPUT_VARIABLE GIT_TOP_LEVEL
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	endif()
	# Note: need to use add_custom_command instead of configure_file to execute at build-time.
	add_custom_command(
		COMMAND "${CMAKE_COMMAND}" "-D OUTPUT=${output}" -P "${script}"
		DEPENDS "${script}" "$<IF:$<BOOL:Git_FOUND>,${GIT_TOP_LEVEL}/.git/index,>"
		OUTPUT  "${output}"
		VERBATIM
	)
	# execute at configure time in case we also need to get/set properties of the file or read it then:
	execute_process(COMMAND "${CMAKE_COMMAND}" "-D OUTPUT=${output}" -P "${script}")

	target_sources(okiidoku PRIVATE "${output}")
endblock()