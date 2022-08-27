# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later

# https://github.com/pybind/cmake_example
# https://pybind11.readthedocs.io/en/stable/cmake/index.html

pybind11_add_module(okiidoku_py MODULE okiidoku.pybind.cpp)
# set_target_properties(okiidoku_py PROPERTIES OUTPUT_NAME "okiidoku") # TODO.try
okiidoku_add_compiler_options(okiidoku_py)
target_link_libraries(okiidoku_py
	PRIVATE okiidoku
)