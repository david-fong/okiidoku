# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later

# https://nanobind.readthedocs.io/en/latest/api_cmake.html
# https://nanobind.readthedocs.io/en/latest/faq.html#how-can-i-reduce-build-time


# https://nanobind.readthedocs.io/en/latest/api_cmake.html#high-level-interface
nanobind_add_module(
	okiidoku_nb # https://nanobind.readthedocs.io/en/latest/faq.html#importing-my-module-fails-with-an-importerror
	${NB_STABLE_ABI}
	NB_SHARED # use shared because I feel like it (better build time?)
	NB_SUPPRESS_WARNINGS
	# LTO # defaults from the CMAKE_... variable
	okiidoku.nanobind.cpp
)
set_target_properties(okiidoku_nb PROPERTIES
	OUTPUT_NAME "okiidoku" # doesn't work. nanobind chokes. :'(
	EXPORT_NAME nanobind
)
okiidoku_add_compiler_options(okiidoku_nb)
target_link_libraries(okiidoku_nb
	PRIVATE okiidoku
)
include(GNUInstallDirs)
okiidoku_install_target(okiidoku_nb DESTINATION "${CMAKE_INSTALL_LIBDIR}")

block()
set(output "okiidoku.pyi")
if(DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
	set(output "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${output}")
endif()
# https://nanobind.readthedocs.io/en/latest/api_cmake.html#stub-generation
# https://nanobind.readthedocs.io/en/latest/typing.html#stubs
# nanobind_add_stub(okiidoku_nb_stub
# 	MODULE okiidoku_nb
# 	OUTPUT "${output}"
# 	# OUTPUT "$<TARGET_FILE_DIR:okiidoku_nb>/okiidoku.pyi" # TODO.wait https://gitlab.kitware.com/cmake/cmake/-/issues/21364
# 	PYTHON_PATH "$<TARGET_FILE_DIR:okiidoku_nb>" # supposed to be relative path?
# 	DEPENDS
# 		okiidoku_nb
# 		okiidoku # for good measure
# 		okiidoku::compiler_warnings # necessary for nanobind v2.7.0's very basic sanitizer detection. I attach sanitizer flags to this target.
# )
endblock()


set(src "$<TARGET_FILE_DIR:okiidoku_nb>")
# set(dest "${CMAKE_CURRENT_SOURCE_DIR}/examples/build/$<CONFIG>")
install(FILES "${src}/okiidoku.pyi" TYPE LIB COMPONENT okiidoku_development OPTIONAL)
# TODO the other files to install. and maybe add convenience symlinks in examples dir like we have for emscripten? or are we going to get rid of those?
