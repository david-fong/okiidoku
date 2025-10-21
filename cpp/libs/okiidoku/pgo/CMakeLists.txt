# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# TODO.wait https://github.com/vector-of-bool/pitchfork/issues/45
if(_OKIIDOKU_BUILD_IS_PGO_GEN)
	add_executable(okiidoku_pgo_trainer_default
		default.main.cpp
	)
	okiidoku_add_compiler_options(okiidoku_pgo_trainer_default)
	target_link_libraries(okiidoku_pgo_trainer_default
		PRIVATE okiidoku_cli_utils
		PRIVATE okiidoku
	)
	install(TARGETS okiidoku_pgo_trainer_default
		EXPORT okiidoku_targets
		COMPONENT okiidoku_pgo_trainers
	)
endif()

cmake_language(DEFER
	DIRECTORY "${okiidoku_SOURCE_DIR}"
	CALL okiidoku_target_pgo
		#[[trainee:]] okiidoku
		#[[trainer:]] okiidoku_pgo_trainer_default
)