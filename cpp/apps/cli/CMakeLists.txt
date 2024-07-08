# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
add_executable(okiidoku_cli
	src/okiidoku_cli/config.cpp
	src/okiidoku_cli/repl.cpp
	src/okiidoku_cli/cli.main.cpp
)
set_target_properties(okiidoku_cli PROPERTIES
	EXPORT_NAME cli
	INTERPROCEDURAL_OPTIMIZATION "${okiidoku_is_ipo_supported}"
)
okiidoku_install_target(okiidoku_cli OPTIONAL)

target_include_directories(okiidoku_cli PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

okiidoku_add_compiler_options(okiidoku_cli)
target_link_libraries(okiidoku_cli
	PRIVATE okiidoku_cli_utils
	PRIVATE okiidoku
)