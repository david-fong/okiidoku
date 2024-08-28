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
		_okiidoku_nb # https://nanobind.readthedocs.io/en/latest/faq.html#importing-my-module-fails-with-an-importerror
		${nb_stable_abi} NB_SHARED ${nb_lto} # use shared because I feel like it (better build time?)
		okiidoku.nanobind.cpp
	)
endblock()
okiidoku_add_compiler_options(_okiidoku_nb) # TODO.wait once nanobind adds its headers as system include directories
target_link_libraries(_okiidoku_nb
	PRIVATE okiidoku
)
include(GNUInstallDirs)
okiidoku_install_target(_okiidoku_nb DESTINATION "${CMAKE_INSTALL_LIBDIR}")

if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
	message(FATAL_ERROR "I do not have the patience to gracefully handle this right now") # TODO.wait see below OUTPUT argument comment
endif()
# https://nanobind.readthedocs.io/en/latest/api_cmake.html#stub-generation
# https://nanobind.readthedocs.io/en/latest/typing.html#stubs
# TODO this is having issues when combined with sanitizers. I somehow worked around it on GCC ("ASAN_OPTIONS": "verify_asan_link_order=false" in dev debug build preset), but can't figure out what to do on Clang.
# nanobind_add_stub(okiidoku_nb_stub
# 	MODULE _okiidoku_nb
# 	OUTPUT "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/okiidoku.pyi"
# 	# OUTPUT "$<TARGET_FILE_DIR:_okiidoku_nb>/okiidoku.pyi" # TODO.wait https://gitlab.kitware.com/cmake/cmake/-/issues/21364
# 	PYTHON_PATH "$<TARGET_FILE_DIR:_okiidoku_nb>" # supposed to be relative path?
# 	DEPENDS _okiidoku_nb
# )
# set_target_properties(_okiidoku_nb PROPERTIES OUTPUT_NAME "okiidoku") # TODO.try

block()
	set(src "$<TARGET_FILE_DIR:_okiidoku_nb>")
	# set(dest "${CMAKE_CURRENT_SOURCE_DIR}/examples/build/$<CONFIG>")
	install(FILES "${src}/okiidoku.pyi" TYPE LIB COMPONENT okiidoku_development)
	# TODO the other files to install. and maybe add convenience symlinks in examples dir like we have for emscripten? or are we going to get rid of those?
endblock()