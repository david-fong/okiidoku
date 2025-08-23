# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 4.1)
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

if(NOT EXISTS "${trainee_binary}")
	message(FATAL_ERROR "PGO: trainee binary was not found: '${trainee_binary}'")
endif()
if(NOT EXISTS "${trainer_binary}")
	message(FATAL_ERROR "PGO: trainer binary was not found: '${trainer_binary}'")
endif()


# short circuit if neither trainee nor trainer have changed since last training run
if(
	EXISTS "${training_stamp_file}"
	AND "${training_stamp_file}" IS_NEWER_THAN "${trainee_binary}"
	AND "${training_stamp_file}" IS_NEWER_THAN "${trainer_binary}"
)
	message(STATUS "PGO: no need to retrain '${trainee}'")
	return() # no need to re-train
endif()
message(STATUS "PGO: train '${trainee}' using '${trainer}'")


if(NOT "${data_dir}" STREQUAL "")
	message(STATUS "     removing any old PGO data")
	file(REMOVE_RECURSE "${data_dir}")
endif()
file(MAKE_DIRECTORY "${data_dir}")


message(STATUS "     executing '${trainer}'")
execute_process(COMMAND "${trainer_binary}"
	WORKING_DIRECTORY "${data_dir}"
	RESULT_VARIABLE exit_code
)
if(NOT "${exit_code}" EQUAL "0")
	message(FATAL_ERROR "PGO: '${trainer}' exited with code ${exit_code}")
endif()


# clang PGO post-processing:
if(DEFINED "CACHE{llvm_profdata}")
	file(GLOB profraw_files LIST_DIRECTORIES NO "${data_dir}/*")
	message(STATUS "PGO: merging ${profraw_files} to ${data_file_for_clang}")
	execute_process(
		COMMAND "${llvm_profdata}" merge "--output=${data_file_for_clang}" "${profraw_files}"
		WORKING_DIRECTORY "${data_dir}"
		RESULT_VARIABLE prof_merge_exit_code
	)
	if(NOT "${prof_merge_exit_code}" EQUAL "0")
		message(FATAL_ERROR "PGO: failed to merge profraw files")
	endif()
endif()


string(TIMESTAMP timestamp "%s" UTC)
file(WRITE "${training_stamp_file}" "// UTC: ${timestamp}")
# TODO if ccache sees through this, figure something else out. maybe an extern variable?
#  maybe the config_extra_files_to_hash option can help