# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# https://cmake.org/cmake/help/latest/command/target_sources.html
target_sources(okiidoku PRIVATE
	o2_bit_arr.cpp
	grid.cpp
	print_2d.cpp

	gen.cpp

	serdes.cpp
	archive.cpp

	morph/transform.cpp
	morph/scramble.cpp
	morph/rel_info.cpp
	morph/canon_label.cpp
	morph/canon_place.cpp
	morph/canon.cpp

	puzzle/ua_set.cpp

	puzzle/solver/engine.cpp
	puzzle/solver/cand_elim_find.guess.cpp
	puzzle/solver/cand_elim_find.subsets.cpp
	puzzle/solver/cand_elim_find.intersections.cpp
	puzzle/solver/cand_elim_find.fish.cpp
	puzzle/solver/cand_elim_apply.cpp
	puzzle/solve.cpp

	puzzle/make.cpp
)


target_sources(okiidoku PRIVATE
	${okiidoku_SOURCE_DIR}/tools/okiidoku.natvis
)