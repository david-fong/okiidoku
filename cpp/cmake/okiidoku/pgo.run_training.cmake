# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 4.2)
if(CMAKE_SCRIPT_MODE_FILE STREQUAL "")
	message(FATAL_ERROR "this should be called as a script")
endif()
# inputs:
# _OKIIDOKU_PGO_DIR
# trainee
# trainer
# trainee_binary
# trainer_binary
# data_dir
# training_stamp_file
# -
# llvm_profdata
# data_file_for_clang

if(NOT EXISTS "$CACHE{trainee_binary}")
	message(FATAL_ERROR "PGO: trainee binary was not found: '$CACHE{trainee_binary}'")
endif()
if(NOT EXISTS "$CACHE{trainer_binary}")
	message(FATAL_ERROR "PGO: trainer binary was not found: '$CACHE{trainer_binary}'")
endif()


# short circuit if neither trainee nor trainer have changed since last training run
if(
	EXISTS "$CACHE{training_stamp_file}"
	AND "$CACHE{training_stamp_file}" IS_NEWER_THAN "$CACHE{trainee_binary}"
	AND "$CACHE{training_stamp_file}" IS_NEWER_THAN "$CACHE{trainer_binary}"
)
	message(STATUS "PGO: no need to retrain '$CACHE{trainee}'")
	return() # no need to re-train
endif()
message(STATUS "PGO: train '$CACHE{trainee}' using '$CACHE{trainer}'")


if(NOT "$CACHE{data_dir}" STREQUAL "")
	message(STATUS "     removing any old PGO data")
	file(REMOVE_RECURSE "$CACHE{data_dir}")
endif()
file(MAKE_DIRECTORY "$CACHE{data_dir}")


message(STATUS "     executing '$CACHE{trainer}'")
execute_process(COMMAND "$CACHE{trainer_binary}"
	WORKING_DIRECTORY "$CACHE{data_dir}"
	RESULT_VARIABLE exit_code
)
if(NOT "${exit_code}" EQUAL "0")
	message(FATAL_ERROR "PGO: '$CACHE{trainer}' exited with code ${exit_code}")
endif()


# clang PGO post-processing:
if(DEFINED CACHE{llvm_profdata})
	file(GLOB profraw_files LIST_DIRECTORIES NO "$CACHE{data_dir}/*")
	message(STATUS "PGO: merging ${profraw_files} to ${data_file_for_clang}")
	execute_process(
		COMMAND "$CACHE{llvm_profdata}" merge "--output=$CACHE{data_file_for_clang}" "${profraw_files}"
		WORKING_DIRECTORY "$CACHE{data_dir}"
		RESULT_VARIABLE prof_merge_exit_code
	)
	if(NOT "${prof_merge_exit_code}" EQUAL "0")
		message(FATAL_ERROR "PGO: failed to merge profraw files")
	endif()
endif()


string(TIMESTAMP timestamp "%s" UTC)
file(WRITE "$CACHE{training_stamp_file}" "// UTC: ${timestamp}")
# TODO if ccache sees through this, figure something else out. maybe an extern variable?
#  maybe the config_extra_files_to_hash option can help