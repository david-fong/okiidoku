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
	morph/canon_sym.cpp
	morph/canon_pos.cpp
	morph/canon.cpp

	puzzle/ua_set.cpp

	puzzle/solver/engine.cpp
	puzzle/solver/cand_elim_find.guess.cpp
	puzzle/solver/cand_elim_find.subsets.cpp
	puzzle/solver/cand_elim_find.intersections.cpp
	puzzle/solver/cand_elim_find.fish.cpp
	puzzle/solver/cand_elim_apply.cpp
	puzzle/solve.cpp

	# puzzle2/solver/cands.cpp

	puzzle/make.cpp
)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_sources(okiidoku PRIVATE
		"$<${debug_configs}:${CMAKE_CURRENT_SOURCE_DIR}/debug/gdb_support.cpp>"
		"$<${debug_configs}:${OKIIDOKU_DATA_OUTPUT_DIRECTORY}/okiidoku.gdb.py>" # draw dependency for custom command
	)
endif()


# tell VS the PDB exists (https://discourse.cmake.org/t/7316/2):
target_sources(okiidoku PRIVATE "${okiidoku_SOURCE_DIR}/tools/okiidoku.natvis") # TODO.low is this needed, given the below link option?
if(MSVC)
	# embed natvis in PDB https://gitlab.kitware.com/cmake/cmake/-/issues/16874
	# TODO does doing this make sense? I'm already currently instaling the natvis file. what's the point of doing that if it's embedded in a PDB? shouldn't I just pick one or the other?
	target_link_options(okiidoku PRIVATE "$<${debug_configs}:/NATVIS:${okiidoku_SOURCE_DIR}/tools/okiidoku.natvis>")
	# TODO verify whether this even works or not. see https://gitlab.kitware.com/cmake/cmake/-/issues/23215
	# https://gitlab.kitware.com/cmake/cmake/-/issues/26278#note_1586578
endif()