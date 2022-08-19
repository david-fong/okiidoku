# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
add_executable(okiidoku_translator
	translator.main.cpp
)

target_include_directories(okiidoku_translator PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(okiidoku_translator
	PUBLIC  okiidoku_compile_options_public
	PRIVATE okiidoku_compiler_warnings
	PRIVATE okiidoku_cli_utils
	PRIVATE okiidoku
)