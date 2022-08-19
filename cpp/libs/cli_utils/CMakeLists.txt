# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
add_library(okiidoku_cli_utils STATIC
   src/okiidoku_cli_utils/console_setup.cpp
)
target_include_directories(okiidoku_cli_utils PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
target_link_libraries(okiidoku_cli_utils
   PUBLIC  okiidoku_compile_options_public
   PRIVATE okiidoku_compiler_warnings
)