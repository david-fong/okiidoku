# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later

if(_OKIIDOKU_BUILD_IS_PGO_GEN)
	add_executable(okiidoku_pgo_trainer_default
		default.main.cpp
	)
	okiidoku_add_compiler_options(okiidoku_pgo_trainer_default)
	target_link_libraries(okiidoku_pgo_trainer_default
		PRIVATE okiidoku_cli_utils
		PRIVATE okiidoku
	)
endif()
okiidoku_enable_profile_guided_optimization(okiidoku #[[trainer:]] okiidoku_pgo_trainer_default)

if(_OKIIDOKU_BUILD_IS_PGO_GEN)
	install(
		TARGETS okiidoku_pgo_trainer_default
		EXPORT okiidoku_installation_targets
		COMPONENT okiidoku_pgo_trainers
		INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
	)
endif()