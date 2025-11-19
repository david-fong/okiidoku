# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY) #[[
This file describes a custom build-system config for a speed-optimized
release build with PGO for selected targets. This is _not_ the same as
providing an option to "add" PGO support to all generated build-system
configs. Rationale for design: I don't see myself ever wanting PGO for
_anything but_ a speed-optimized release build.

https://gitlab.kitware.com/cmake/cmake/-/issues/19273

note/question: when instrumenting a program with dynamically linked components,
	does each component need to have a different profile-data path? I'm guessing yes?
I don't really care about PGO for anything but the core library

TODO look into Ninja Multi-Config "Cross-Config" and see if it can help?
- https://cmake.org/cmake/help/latest/generator/Ninja%20Multi-Config.html
  - https://cmake.org/cmake/help/latest/variable/CMAKE_CROSS_CONFIGS.html
  - https://cmake.org/cmake/help/latest/command/add_custom_target.html#ninja-multi-config
  - https://cmake.org/cmake/help/latest/command/add_custom_command.html#ninja-multi-config
]]

# MARK: input validation and context query
set(_config_types "${CMAKE_CONFIGURATION_TYPES}")
if(DEFINED CMAKE_BUILD_TYPE AND _config_types STREQUAL "")
	# handle if user strangely sets both CMAKE_CONFIGURATION_TYPES and
	# CMAKE_BUILD_TYPE (which I lazily do in the ExternalProject_Add :P)
	list(APPEND _config_types "${CMAKE_BUILD_TYPE}")
endif()
if("PgoGen" IN_LIST _config_types)
	set(_OKIIDOKU_BUILD_IS_PGO_GEN YES)
	if(NOT _config_types STREQUAL "PgoGen") # PgoGen should always be the _only_ generated config for a generated build-system
		message(FATAL_ERROR "The 'PgoGen' configuration is an internal meta-build-system detail and something is wrong. Did you try to use it directly?")
	endif()
	if(NOT DEFINED _OKIIDOKU_PGO_CALLING_CONFIG)
		message(FATAL_ERROR "The 'PgoGen' ExternalProject needs to be passed a `_OKIIDOKU_PGO_CALLING_CONFIG`.")
	endif()
	if(NOT DEFINED _OKIIDOKU_PGO_DIR)
		message(FATAL_ERROR "The 'PgoGen' ExternalProject needs to be passed a `_OKIIDOKU_PGO_DIR`.")
	endif()
else()
	set(_OKIIDOKU_BUILD_IS_PGO_GEN NO)
endif()

# short circuit if user doesn't care about PGO.
if((NOT "PgoUse" IN_LIST _config_types) AND (NOT _OKIIDOKU_BUILD_IS_PGO_GEN))
	unset(_config_types)
	function(okiidoku_target_pgo) # no-op
	endfunction()
	return()
endif()
unset(_config_types)


# MARK: add training project
# cspell:words PGOGEN PGOUSE
# TODO don't I need to do more? https://stackoverflow.com/a/75828118/11107541
list(PREPEND CMAKE_CXX_FLAGS_PGOGEN ${CMAKE_CXX_FLAGS_RELEASE})
list(PREPEND CMAKE_CXX_FLAGS_PGOUSE ${CMAKE_CXX_FLAGS_RELEASE})
if(NOT _OKIIDOKU_BUILD_IS_PGO_GEN)
	set(_OKIIDOKU_PGO_DIR "${okiidoku_BINARY_DIR}/_pgo")
	set(okiidoku_pgo_gen_cmake_generator "${CMAKE_GENERATOR}")
	if(okiidoku_pgo_gen_cmake_generator STREQUAL "Ninja Multi-Config")
		set(okiidoku_pgo_gen_cmake_generator "Ninja")
	endif()

	# TODO find a way to only do this if the config is PgoUse?
	# TODO.low I think things can go out of sync if the cache is manually edited...
	include(ExternalProject)
	ExternalProject_Add(okiidoku_pgo_gen
		# directory options:
		PREFIX "${_OKIIDOKU_PGO_DIR}"
		SOURCE_DIR "${okiidoku_SOURCE_DIR}"
		BINARY_DIR "${_OKIIDOKU_PGO_DIR}/build" # default is `<prefix>/src/<name>-build`
		# configure step options:
		CMAKE_GENERATOR "${okiidoku_pgo_gen_cmake_generator}"
		CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
		CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}"
		CMAKE_GENERATOR_INSTANCE "${CMAKE_GENERATOR_INSTANCE}"
		CMAKE_ARGS
		CMAKE_CACHE_ARGS
			"-D CMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}"
			"-D CMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}"
			"-D CMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}"
			"-D CMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}"
			"-D CMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}"
			"-D CMAKE_CXX_EXTENSIONS:BOOL=${CMAKE_CXX_EXTENSIONS}"
			"-D CMAKE_CONFIGURATION_TYPES:STRING=PgoGen"
			"-D CMAKE_BUILD_TYPE:STRING=PgoGen"
			"-D CMAKE_BUILD_RPATH_USE_ORIGIN:BOOL=${CMAKE_BUILD_RPATH_USE_ORIGIN}"
			"-D CMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${_OKIIDOKU_PGO_DIR}/out"
			"-D CMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${_OKIIDOKU_PGO_DIR}/out"
			"-D CMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${_OKIIDOKU_PGO_DIR}/out"
			"-D CMAKE_EXPORT_COMPILE_COMMANDS:BOOL=YES"
			"-D CPM_SOURCE_CACHE:PATH=${CPM_SOURCE_CACHE}" # TODO.low omit if not defined.
			"-D _OKIIDOKU_PGO_CALLING_CONFIG:INTERNAL=$<CONFIG>"
			"-D _OKIIDOKU_PGO_DIR:INTERNAL=${_OKIIDOKU_PGO_DIR}"
			"-D OKIIDOKU_BUILD_SHARED_LIBS:BOOL=${OKIIDOKU_BUILD_SHARED_LIBS}"
			"-D OKIIDOKU_BUILD_WITH_STATIC_ANALYZERS:BOOL=NO"
				"-D CMAKE_SKIP_LINTING:BOOL=YES" # (redundant?)
			"-D OKIIDOKU_BUILD_DEBUG_WITH_SANITIZERS:BOOL=NO"
			"-D OKIIDOKU_BUILD_REPRODUCIBLE:BOOL=${OKIIDOKU_BUILD_REPRODUCIBLE}"
			"-D OKIIDOKU_BUILD_TESTING:BOOL=NO"
			"-D OKIIDOKU_BUILD_DOCS:BOOL=NO"
			"-D OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE:BOOL=${OKIIDOKU_BUILD_OPTIMIZE_LOCAL_NON_PORTABLE}"
			"-D OKIIDOKU_BUILD_BINDINGS_FOR_PYTHON:BOOL=NO"
		CONFIGURE_HANDLED_BY_BUILD YES
		# build step options:
		BUILD_ALWAYS YES
		BUILD_BYPRODUCTS
		# install step options:
		INSTALL_COMMAND "${CMAKE_COMMAND}" --install . --config PgoGen --prefix <INSTALL_DIR>
		# test step options:
		# output logging options:
		LOG_CONFIGURE YES LOG_BUILD YES LOG_INSTALL YES LOG_TEST YES
		LOG_OUTPUT_ON_FAILURE YES
		# target options:
		EXCLUDE_FROM_ALL YES
		STEP_TARGETS build
		# TODO consider instead ExternalProject_Add_Step in okiidoku_target_pgo that adds a step to `cmake --build --target ${trainer}`
		# misc options:
	)
endif()




# MARK: okiidoku_target_pgo
#[[
add a PGO trainee+trainer pair
\param trainee a target library
\param trainer a target executable that makes use of the trainee- ideally lightweight
\note due to CMake quirks, this must be called in the same directory scope as the one
	defining the okiidoku_pgo_gen ExternalProject. you could use `cmake_language(DEFER ...)`
	if you want to register the trainee/trainer pair in a subdirectory. that also has the
	benefit that it will go after `source_file_random_seed.cmake` worked done its magic.
TODO.low consider making this work for training executables as well?
]]
function(okiidoku_target_pgo
	trainee # name of target to train for PGO
	trainer # name of a executable target to use for training PGO
)
	set(if_pgo "$<CONFIG:PgoGen,PgoUse>")
	set(if_gen "$<CONFIG:PgoGen>")
	set(if_use "$<CONFIG:PgoUse>")
	if(NOT DEFINED "_OKIIDOKU_PGO_DIR" OR "${_OKIIDOKU_PGO_DIR}" STREQUAL "")
		message(FATAL_ERROR "_OKIIDOKU_PGO_DIR is not defined!!!!!!!!!!")
	endif()
	set(trainer_binary "${_OKIIDOKU_PGO_DIR}/out/${trainer}${CMAKE_EXECUTABLE_SUFFIX}")

	if(NOT CMAKE_CURRENT_BINARY_DIR STREQUAL okiidoku_BINARY_DIR)
		# (for CMake reasons I don't fully understand, custom steps for ExternalProject must be registered...)
		message(FATAL_ERROR "okiidoku_target_pgo must be called in the ${okiidoku_SOURCE_DIR} directory scope.")
	endif()
	if(NOT TARGET "${trainee}")
		message(FATAL_ERROR "PGO trainee must be a target, but '${trainee}' is not.")
	endif()
	if(_OKIIDOKU_BUILD_IS_PGO_GEN)
		if(NOT TARGET "${trainer}")
			message(FATAL_ERROR "PGO trainer must be a target, but '${trainer}' is not.")
		endif()
		get_target_property(trainer_target_type ${trainer} TYPE)
		if(NOT trainer_target_type STREQUAL "EXECUTABLE")
			message(FATAL_ERROR "PGO trainer must be an EXECUTABLE target, but '${trainer}' is a ${trainer_target_type}.")
		endif()
		unset(trainer_target_type)
		# TODO: assert that trainer has link dependency on trainee
	else()
		ExternalProject_Add_Step(okiidoku_pgo_gen "build-${trainer}" EXCLUDE_FROM_MAIN YES
			COMMENT "build '${trainer}:PgoGen' (to train '${trainee}:PgoUse')"
			WORKING_DIRECTORY "<BINARY_DIR>"
			COMMAND "${CMAKE_COMMAND}" --build . --config PgoGen --target "${trainer}"
			BYPRODUCTS "$<${if_use}:${trainer_binary}>"
			DEPENDEES configure
			# JOB_SERVER_AWARE YES # ?
			LOG YES
			USES_TERMINAL YES
			ENVIRONMENT_MODIFICATION # TODO inherit "LANG" "LC_ALL" "TZ" ("LC_CTYPE"?)
		)
		ExternalProject_Add_StepTargets(okiidoku_pgo_gen "build-${trainer}")
		# we're doing this weirdness with `BYPRODUCTS` and `mock_build...` because
		# generator expressions aren't supported in `DEPENDS` or `add_dependencies`. sigh.
		add_custom_target("mock_build-${trainer}"
			"${CMAKE_COMMAND}" -E "$<IF:${if_use},true,touch;${trainer_binary}>"
			COMMENT [[🦗]]
			BYPRODUCTS "$<$<NOT:${if_use}>:${trainer_binary}>"
			COMMAND_EXPAND_LISTS
		)
	endif()

	set(data_dir "${_OKIIDOKU_PGO_DIR}/data/${trainer}")
	file(MAKE_DIRECTORY "${data_dir}")

	if(MSVC)
	block()
		# cspell:words LTCG GENPROFILE USEPROFILE
		# https://docs.microsoft.com/en-us/cpp/build/profile-guided-optimizations
		set(data_filename "${trainer}.pgd")
		target_compile_options(${trainee} PRIVATE "$<${if_gen}:/GL>")
		target_link_options(${trainee} PRIVATE
			"$<${if_gen}:/LTCG:INCREMENTAL>"
			"$<${if_gen}:/GENPROFILE:PGD=${data_filename}>"
			"$<${if_use}:/USEPROFILE:PGD=${data_filename}>"
		)
	endblock()
	endif()

	if((CMAKE_CXX_COMPILER_ID MATCHES [[Clang]]) OR EMSCRIPTEN)
		# TODO test this and fix problems
		# cspell:words "-fprofile" instr profdata profraw
		# https://clang.llvm.org/docs/UsersManual.html#profile-guided-optimization
		# https://clang.llvm.org/docs/UsersManual.html#profiling-with-instrumentation
		set(data_file_for_clang "${data_dir}/merged.profdata")
		target_compile_options(${trainee} PRIVATE
			"$<${if_gen}:-fprofile-generate=${data_dir}>"
			"$<${if_use}:-fprofile-use=${data_file_for_clang}>"
			# TODO: try https://clang.llvm.org/docs/UsersManual.html#cmdoption-ftemporal-profile
			# https://clang.llvm.org/docs/UsersManual.html#instrumenting-only-selected-files-or-functions
		)
		target_link_options(${trainee} PRIVATE
			"$<${if_gen}:-fprofile-generate=${data_dir}>"
		)

	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		# https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
		# https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html

		target_compile_options(${trainee} PRIVATE
			"$<${if_pgo}:-fprofile-prefix-path=$<TARGET_INTERMEDIATE_DIR:${trainee}>>"
			"$<${if_gen}:-fprofile-generate=${data_dir}>"
			"$<${if_use}:-fprofile-use=${data_dir}>"
			# "$<${if_use}:-fprofile-correction>
			# "$<${if_use}:-fprofile-partial-training> # for code not run during training, optimize as normal instead of for size. # TODO.asap should we use partial training?
			# -fprofile-reproducible # TODO investigate https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html#index-fprofile-reproducible
			# alternative? https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#index-fauto-profile
		)
		target_link_options(${trainee} PRIVATE
			"$<${if_gen}:-fprofile-generate=${data_dir}>"
		)
		# https://gcc.gnu.org/wiki/AutoFDO/Tutorial

	# elseif(CMAKE_CXX_COMPILER_ID MATCHES [[Intel]])
		# https://www.intel.com/content/www/us/en/develop/documentation/cpp-compiler-developer-guide-and-reference/top/optimization-and-programming/profile-guided-optimization-pgo.html
		# https://www.intel.com/content/www/us/en/develop/documentation/cpp-compiler-developer-guide-and-reference/top/optimization-and-programming/profile-guided-optimization-pgo/profile-an-application-with-instrumentation.html
	endif()

	# the rest of what follows is to make the `PgoUse` config run training.
	# trainer and trainee for the `PgoGen` side are done being configured now.
	if(_OKIIDOKU_BUILD_IS_PGO_GEN)
		return()
	endif()

	# define training timestamp header (marks last training run; used to re-train when trainer/trainee has changed)
	block(PROPAGATE training_stamp_file)
		target_include_directories(${trainee} PRIVATE "$<${if_use}:$<BUILD_INTERFACE:${data_dir}/include>>")
		set(include_subdir "${trainee}/detail/pgo")
		set(timestamp_dir "${data_dir}/include/${include_subdir}")
		file(MAKE_DIRECTORY "${timestamp_dir}")
		set(training_stamp_file "${timestamp_dir}/${trainer}.timestamp.hpp")
		if(NOT EXISTS "${training_stamp_file}")
			file(TOUCH "${training_stamp_file}")
		endif()
		okiidoku_make_include_flag("${include_subdir}/${trainer}.timestamp.hpp" training_stamp_include_flag)
		target_compile_options("${trainee}" PRIVATE "$<${if_use}:$<BUILD_INTERFACE:${training_stamp_include_flag}>>")
		target_sources(${trainee} PRIVATE "$<${if_use}:${training_stamp_file}>") # just to show up in some IDEs
	endblock()

	block()
	# note: annoyingly, commands cannot be the empty string. use `cmake -E true` as a no-op instead.
	list(APPEND command
		"${CMAKE_COMMAND}"
		"-D _OKIIDOKU_PGO_DIR=${_OKIIDOKU_PGO_DIR}"
		"-D trainee=${trainee}"
		"-D trainer=${trainer}"
		"-D trainee_binary=${_OKIIDOKU_PGO_DIR}/out/$<TARGET_FILE_NAME:${trainee}>"
		"-D trainer_binary=${trainer_binary}"
		"-D data_dir=${data_dir}"
		"-D training_stamp_file=${training_stamp_file}"
	)
	set(byproducts "${training_stamp_file}")
	if((CMAKE_CXX_COMPILER_ID MATCHES [[Clang]])) # TODO `OR EMSCRIPTEN`, which has ${EMSDK}/upstream/bin/llvm-profdata
		if(NOT DEFINED CMAKE_CXX_COMPILER_VERSION)
			message(WARNING "could not get llvm version from `CMAKE_CXX_COMPILER_VERSION`")
			return()
		endif()
		string(REPLACE "." ";" llvm_version_list "${CMAKE_CXX_COMPILER_VERSION}")
		list(GET llvm_version_list 0 llvm_major_version)
		find_program(LLVM_PROFDATA "llvm-profdata-${llvm_major_version}")
		# https://llvm.org/docs/CommandGuide/llvm-profdata.html#profdata-merge
		list(APPEND command
			"-D llvm_profdata=${LLVM_PROFDATA}"
			"-D data_file_for_clang=${data_file_for_clang}"
		)
		list(APPEND byproducts "${data_file_for_clang}")
	endif()
	list(APPEND command "-P" "${okiidoku_SOURCE_DIR}/cmake/okiidoku/pgo.run_training.cmake")
	add_custom_target("run_${trainer}"
		COMMAND "$<IF:${if_use},${command},${CMAKE_COMMAND};-E;true>"
		DEPENDS "${trainer_binary}"
		BYPRODUCTS "$<IF:${if_use},${byproducts},>"
		WORKING_DIRECTORY "$<IF:${if_use},${data_dir},${okiidoku_SOURCE_DIR}>"
		COMMENT "$<IF:${if_use},PGO: checking if '${trainer}' needs to be re-run to train '${trainee}',>"
		VERBATIM COMMAND_EXPAND_LISTS
	)
	endblock()
	# add_dependencies("run_${trainer}" "okiidoku_pgo_gen-build-${trainer}") # must build trainee before training
	# build will stop if run_trainer fails
	# TODO.wait use generator expression once `add_dependencies` supports them (so dependency only exists for PgoUse config). https://gitlab.kitware.com/cmake/cmake/-/issues/19467. currently doing workaround in the custom step COMMANDs.
	add_dependencies(${trainee} "run_${trainer}")

endfunction()