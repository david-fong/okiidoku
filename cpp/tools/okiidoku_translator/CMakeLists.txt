# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
add_executable(okiidoku_translator
	translator.main.cpp
)

target_include_directories(okiidoku_translator PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
okiidoku_add_compiler_options(okiidoku_translator)
target_link_libraries(okiidoku_translator
	PRIVATE okiidoku_cli_utils
	PRIVATE okiidoku
)