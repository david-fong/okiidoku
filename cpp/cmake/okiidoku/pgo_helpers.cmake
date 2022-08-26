# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)

# This file describes a custom build-system config for a speed-optimized
# release build with PGO for selected targets. This is _not_ the same as
# providing an option to "add" PGO support to all generated build-system
# configs. Rationale for design: I don't see myself ever wanting PGO for
# _anything but_ a speed-optimized release build.

# https://gitlab.kitware.com/cmake/cmake/-/issues/19273

# Note/question: when instrumenting a program with dynamically linked components,
#  does each component need to have a different profile-data path? I'm guessing yes?
# I don't really care about PGO for anything but the core library

set(_config_types "${CMAKE_CONFIGURATION_TYPES}")
if(DEFINED CMAKE_BUILD_TYPE AND _config_types STREQUAL "")
	# handle if user strangely sets both CMAKE_CONFIGURATION_TYPES and
	# CMAKE_BUILD_TYPE (which I lazily do in the ExternalProject_Add :P)
	list(APPEND _config_types "${CMAKE_BUILD_TYPE}")
endif()

if("PgoGen" IN_LIST _config_types)
	set(_OKIIDOKU_BUILD_IS_PGO_GEN YES)
endif()

if(_OKIIDOKU_BUILD_IS_PGO_GEN AND (NOT _config_types STREQUAL "PgoGen"))
	# PgoGen should always be the only generated config for a generated build-system
	message(FATAL_ERROR "The \"PgoGen\" configuration is an internal meta-build-system detail and something is wrong. Did you try to use it directly?")
endif()
if(_OKIIDOKU_BUILD_IS_PGO_GEN AND NOT DEFINED _OKIIDOKU_PGO_CALLING_CONFIG)
	message(FATAL_ERROR "The \"PgoGen\" ExternalProject needs to be passed a `_OKIIDOKU_PGO_CALLING_CONFIG`.")
endif()
if(_OKIIDOKU_BUILD_IS_PGO_GEN AND NOT DEFINED _OKIIDOKU_PGO_DATA_ROOT_DIR)
	message(FATAL_ERROR "The \"PgoGen\" ExternalProject needs to be passed a `_OKIIDOKU_PGO_DATA_ROOT_DIR`.")
endif()

# Note: wrap with `macro` so `return()` returns from caller instead of this `include()`ed file.
# This should be called right after `include()`ing this file.
macro(okiidoku_pgo_gen_check_needs_short_circuit)
	if(DEFINED _OKIIDOKU_PGO_CALLING_CONFIG AND NOT _OKIIDOKU_PGO_CALLING_CONFIG STREQUAL "PgoUse")
		return() # I couldn't think of a better way to "short-circuit" the `ExternalProject_Add`
	endif()
endmacro()

if((NOT "PgoUse" IN_LIST _config_types) AND (NOT _OKIIDOKU_BUILD_IS_PGO_GEN))
	unset(_config_types)
	return() # short circuit if user doesn't care about PGO.
endif()
unset(_config_types)

# cspell:words PGOGEN PGOUSE
list(PREPEND CMAKE_CXX_FLAGS_PGOGEN ${CMAKE_CXX_FLAGS_RELEASE})
list(PREPEND CMAKE_CXX_FLAGS_PGOUSE ${CMAKE_CXX_FLAGS_RELEASE})
if(NOT _OKIIDOKU_BUILD_IS_PGO_GEN)
	set(_OKIIDOKU_PGO_DATA_ROOT_DIR "${PROJECT_BINARY_DIR}/_pgo")
endif()


# this must be called in the same directory scope as the one defining the trainee
# trainee a target shared library
# trainer a target executable- ideally lightweight
# TODO.low consider making this work for training executables as well?
function(okiidoku_enable_profile_guided_optimization
	trainee # name of target to train for PGO
	trainer # name of a executable target to use for training PGO
)
	if(_OKIIDOKU_BUILD_IS_PGO_GEN)
		get_target_property(trainer_target_type ${trainer} TYPE)
		if(NOT trainer_target_type STREQUAL "EXECUTABLE")
			message(FATAL_ERROR "pgo trainer must be an EXECUTABLE target, but \"${trainer}\" is a ${trainer_target_type}")
		endif()
		unset(trainer_target_type)
	endif()

	set(if_pgo "$<CONFIG:PgoGen,PgoUse>")
	set(if_gen "$<CONFIG:PgoGen>")
	set(if_use "$<CONFIG:PgoUse>")

	if(MSVC)
		# cspell:words LTCG GENPROFILE USEPROFILE
		# https://docs.microsoft.com/en-us/cpp/build/profile-guided-optimizations
		set(data_filename "${trainee}__trained_by__${trainer}.pgd")
		target_compile_options(${trainee} PRIVATE "$<${if_gen}:/GL>")
		target_link_options(${trainee} PRIVATE
			"$<${if_gen}:/LTCG:INCREMENTAL>"
			"$<${if_gen}:/GENPROFILE:PGD=${data_filename}>"
			"$<${if_use}:/USEPROFILE:PGD=${data_filename}>"
		)
		unset(data_filename)
	endif()


	set(data_dir "${_OKIIDOKU_PGO_DATA_ROOT_DIR}/${trainee}/trained_by/${trainer}")

	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR EMSCRIPTEN)
		# cspell:words "-fprofile" instr profdata profraw
		# https://clang.llvm.org/docs/UsersManual.html#profile-guided-optimization
		# https://clang.llvm.org/docs/UsersManual.html#profiling-with-instrumentation
		set(profdata_file "${data_dir}/${trainer}.profdata")
		target_compile_options(${trainee} PRIVATE
			"$<${if_gen}:-fprofile-instr-generate=${data_dir}>"
			"$<${if_use}:-fprofile-instr-use=${profdata_file}>"
		)
		target_link_options(${trainee} PRIVATE
			"$<${if_gen}:-fprofile-generate=${data_dir}>"
		)
		if(_OKIIDOKU_BUILD_IS_PGO_GEN)
			if(NOT DEFINED CMAKE_CXX_COMPILER_VERSION)
				message(WARNING "could not get llvm version from `CMAKE_CXX_COMPILER_VERSION`")
				return()
			endif()
			string(REPLACE llvm_version "." ";" llvm_version_list "${CMAKE_CXX_COMPILER_VERSION}")
			list(GET llvm_version_list 0 llvm_major_version)
			find_program(LLVM_PROFDATA "llvm-profdata-${llvm_major_version}")
			add_custom_command(TARGET ${trainee} PRE_BUILD VERBATIM COMMAND_EXPAND_LISTS
				DEPENDS ""
				COMMAND "$<$<CONFIG:PgoGen>:${LLVM_PROFDATA};merge;-output=\"${profdata_file}\";${data_dir}/*.profraw>"
				BYPRODUCTS "${profdata_file}"
			)
			unset(LLVM_PROFDATA)
			unset(llvm_version_major)
			unset(llvm_version_list)
		endif()
		unset(profdata_file)


	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		# https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
		# https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
		# TODO issue warning about -fprofile-prefix-path if generator has not been tested by me.
		get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
		if(is_multi_config)
			set(int_dir "$<CONFIG>")
		else()
			set(int_dir ".")
		endif()
		unset(is_multi_config)
		get_target_property(trainee_binary_dir ${trainee} BINARY_DIR)
		set(objects_dir "${trainee_binary_dir}/CMakeFiles/${trainee}.dir/${int_dir}")
		unset(trainee_binary_dir)
		unset(int_dir)
		target_compile_options(${trainee} PRIVATE
			"$<${if_pgo}:-fprofile-dir=${data_dir}>"
			"$<${if_pgo}:-fprofile-prefix-path=${objects_dir}>"
			"$<${if_gen}:-fprofile-generate=${data_dir}>"
			"$<${if_use}:-fprofile-use=${data_dir}>"
			# "$<${if_use}:-fprofile-correction>
			# "$<${if_use}:-fprofile-partial-training> # for code not run during training, optimize as normal instead of for size.
		)
		target_link_options(${trainee} PRIVATE
			"$<${if_gen}:-fprofile-generate=${data_dir}>"
		)
		# https://gcc.gnu.org/wiki/AutoFDO/Tutorial


	# elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
		# https://www.intel.com/content/www/us/en/develop/documentation/cpp-compiler-developer-guide-and-reference/top/optimization-and-programming/profile-guided-optimization-pgo.html
		# https://www.intel.com/content/www/us/en/develop/documentation/cpp-compiler-developer-guide-and-reference/top/optimization-and-programming/profile-guided-optimization-pgo/profile-an-application-with-instrumentation.html
	endif()


	if(_OKIIDOKU_BUILD_IS_PGO_GEN)
		return()
	endif()

	set(training_generator "${CMAKE_GENERATOR}")
	if(training_generator STREQUAL "Ninja Multi-Config")
		set(training_generator "Ninja")
	endif()

	# cspell:words "-DCMAKE" "-DOKIIDOKU"
	# TODO find a way to only do this if the config is PgoUse?
	set(training_proj "${trainee}_pgo_training")
	include(ExternalProject)
	ExternalProject_Add("${training_proj}"
		# directory options:
		PREFIX "${CMAKE_BINARY_DIR}/_pgo"
		SOURCE_DIR "${PROJECT_SOURCE_DIR}"
		# configure step options:
		CMAKE_GENERATOR "${training_generator}"
		CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
		CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}"
		CMAKE_GENERATOR_INSTANCE "${CMAKE_GENERATOR_INSTANCE}"
		CMAKE_ARGS
		CMAKE_CACHE_ARGS
			"-D CMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}"
			"-D CMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}"
			"-D CMAKE_CXX_EXTENSIONS:BOOL=${CMAKE_CXX_EXTENSIONS}"
			"-D CMAKE_CONFIGURATION_TYPES:STRING=PgoGen"
			"-D CMAKE_BUILD_TYPE:STRING=PgoGen"
			"-D CMAKE_EXPORT_COMPILE_COMMANDS:BOOL=YES"
			"-D CPM_SOURCE_CACHE:PATH=${CPM_SOURCE_CACHE}" # TODO.low omit if not defined.
			"-D _OKIIDOKU_PGO_CALLING_CONFIG:INTERNAL=$<CONFIG>"
			"-D _OKIIDOKU_PGO_DATA_ROOT_DIR:INTERNAL=${_OKIIDOKU_PGO_DATA_ROOT_DIR}"
			"-D OKIIDOKU_BUILD_TESTING:BOOL=NO"
			"-D OKIIDOKU_BUILD_SHARED:BOOL=${OKIIDOKU_BUILD_SHARED_LIBS}"
		CONFIGURE_HANDLED_BY_BUILD YES
		# build step options:
		# BUILD_ALWAYS YES
		# install step options:
		INSTALL_COMMAND "" # no-op
		# test step options:
		#  none
		# output logging options:
		LOG_CONFIGURE YES LOG_BUILD YES LOG_INSTALL YES LOG_TEST YES
		LOG_OUTPUT_ON_FAILURE YES
		# target options:
		EXCLUDE_FROM_ALL YES
		STEP_TARGETS build install
	)
	ExternalProject_Add_Step("${training_proj}" train
		COMMAND "$<${if_use}:<INSTALL_DIR>/${CMAKE_INSTALL_BINDIR}/${trainer}>"
		# COMMENT "running the PGO training program (\"${trainer}\") for \"${trainee}\"..."
		DEPENDEES install
		# DEPENDERS
		# BYPRODUCTS
	)
	ExternalProject_Add_StepTargets("${training_proj}" train)
	# TODO.wait use generator expression once `add_dependencies` supports them. https://gitlab.kitware.com/cmake/cmake/-/issues/19467
	add_dependencies(${trainee} "${training_proj}-train")

	# how do I want to run the training program?
	# - configure it to be a test and use the test step functions of ExternalProject
	# - use find_package to import the external project and run it that way?
	# - use STEP_TARGETS or ExternalProject_Add_StepTargets() to create a custom target and then add_dependencies()
endfunction()