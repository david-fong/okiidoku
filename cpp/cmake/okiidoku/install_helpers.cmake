# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:words CMAKEDIR
include_guard(DIRECTORY)
include(GNUInstallDirs)

if(NOT DEFINED OKIIDOKU_INSTALL_CMAKEDIR)
	set(OKIIDOKU_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/okiidoku")
endif()

function(okiidoku_install_target target)
	install(
		TARGETS ${target}
		EXPORT okiidoku_installation_targets
		# RUNTIME_DEPENDENCIES # currently there are none to bundle.
		${ARGN}
		RUNTIME  COMPONENT okiidoku_runtime
		LIBRARY  COMPONENT okiidoku_runtime NAMELINK_COMPONENT okiidoku_development
		ARCHIVE  COMPONENT okiidoku_development
		INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
	)
endfunction()