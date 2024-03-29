# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.22)
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
if(NOT EXISTS "${trainee_binary}" OR NOT EXISTS "${trainer_binary}")
	message(FATAL_ERROR "trainee or trainer binaries were not found.")
endif()
if(
	EXISTS "${training_stamp_file}"
	AND "${training_stamp_file}" IS_NEWER_THAN "${trainee_binary}"
	AND "${training_stamp_file}" IS_NEWER_THAN "${trainer_binary}"
)
	return() # no need to re-train
endif()

if(NOT data_dir STREQUAL "")
	message(STATUS "removing any old PGO data")
	file(REMOVE_RECURSE "${data_dir}")
endif()
file(MAKE_DIRECTORY "${data_dir}")

message(STATUS "executing '${trainer}'")
execute_process(COMMAND "${trainer_binary}"
	WORKING_DIRECTORY "${data_dir}"
	RESULT_VARIABLE exit_code
)
if(NOT exit_code EQUAL 0)
	message(FATAL_ERROR "${trainer} exited with code ${exit_code}")
endif()

string(TIMESTAMP timestamp "%s" UTC)
file(WRITE "${training_stamp_file}" "// UTC: ${timestamp}")