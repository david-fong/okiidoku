# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later

# https://nanobind.readthedocs.io/en/latest/api_cmake.html
# https://nanobind.readthedocs.io/en/latest/faq.html#how-can-i-reduce-build-time

block()
if(NOT OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE)
	set(nb_stable_abi STABLE_API) # stable across minor python versions. small perf cost.
endif()
if(okiidoku_is_ipo_supported)
	set(nb_lto LTO)
endif()
# https://nanobind.readthedocs.io/en/latest/api_cmake.html#high-level-interface
nanobind_add_module(
	okiidoku_py # https://nanobind.readthedocs.io/en/latest/faq.html#importing-my-module-fails-with-an-importerror
	${nb_stable_abi} NB_SHARED ${nb_lto} # use shared because I feel like it (better build time?)
	okiidoku.nanobind.cpp
)
endblock()
# okiidoku_add_compiler_options(okiidoku_py) # TODO.wait once nanobind adds its headers as system include directories
target_link_libraries(okiidoku_py
	PRIVATE okiidoku
)
okiidoku_install_target(okiidoku_py)

if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
	message(FATAL_ERROR "I do not have the patience to gracefully handle this right now") # TODO.wait see below OUTPUT argument comment
endif()
# https://nanobind.readthedocs.io/en/latest/api_cmake.html#stub-generation
# https://nanobind.readthedocs.io/en/latest/typing.html#stubs
nanobind_add_stub(okiidoku_py_stub
	MODULE okiidoku_py
	OUTPUT "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/okiidoku_py.pyi"
	# OUTPUT "$<TARGET_FILE_DIR:okiidoku_py>/okiidoku.pyi" # TODO.wait https://gitlab.kitware.com/cmake/cmake/-/issues/21364
	PYTHON_PATH "$<TARGET_FILE_DIR:okiidoku_py>" # supposed to be relative path?
	DEPENDS okiidoku_py
)
# set_target_properties(okiidoku_py PROPERTIES OUTPUT_NAME "okiidoku") # TODO.try

include(GNUInstallDirs)
block()
set(src "$<TARGET_FILE_DIR:okiidoku_py>")
# set(dest "${CMAKE_CURRENT_SOURCE_DIR}/examples/build/$<CONFIG>")
install(FILES "${src}/okiidoku.pyi" TYPE LIB COMPONENT okiidoku_development)
# TODO the other files to install. and maybe convenience symlinks in examples dir like we have for emscripten? or are we going to get rid of those?
endblock()