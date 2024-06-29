# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later

# https://nanobind.readthedocs.io/en/latest/api_cmake.html
# https://nanobind.readthedocs.io/en/latest/faq.html#how-can-i-reduce-build-time

block()
if(BUILD_SHARED_LIBS)
	set(nb_shared_libs NB_SHARED)
else()
	set(nb_shared_libs NB_STATIC)
endif()
if(NOT OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE)
	set(nb_stable_abi STABLE_API)
else()
endif()
if(okiidoku_is_ipo_supported)
	set(nb_lto LTO)
endif()

# https://nanobind.readthedocs.io/en/latest/api_cmake.html#high-level-interface
nanobind_add_module(
	okiidoku_py # https://nanobind.readthedocs.io/en/latest/faq.html#importing-my-module-fails-with-an-importerror
	${nb_stable_abi} "${nb_shared_libs}" ${nb_lto}
	okiidoku.nanobind.cpp
)
# https://nanobind.readthedocs.io/en/latest/api_cmake.html#stub-generation
# https://nanobind.readthedocs.io/en/latest/typing.html#stubs
nanobind_add_stub(okiidoku_py_stub
	MODULE okiidoku_py
	OUTPUT "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/okiidoku_py.pyi"
	# OUTPUT "$<PATH:GET_PARENT_PATH,$<TARGET_FILE:okiidoku_py>>/okiidoku.pyi" # TODO having issues with this
	PYTHON_PATH "$<TARGET_FILE_DIR:okiidoku_py>" # supposed to be relative path?
	DEPENDS okiidoku_py
)
endblock()
# set_target_properties(okiidoku_py PROPERTIES OUTPUT_NAME "okiidoku") # TODO.try

# okiidoku_add_compiler_options(okiidoku_py) # TODO.wait once nanobind adds its headers as system include directories
target_link_libraries(okiidoku_py
	PRIVATE okiidoku
)