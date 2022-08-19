# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
# cspell:words "-fprofile" instr
include_guard(DIRECTORY)
if("PgoUse" NOT IN_LIST CMAKE_CONFIGURATION_TYPES)
	return()
endif()
if("PgoGen" IN_LIST CMAKE_CONFIGURATION_TYPES AND CMAKE_CONFIGURATION_TYPES NOT STREQUAL "PgoGen")
	message(FATAL_ERROR "The PgoGen configuration is an internal build-system detail and something is wrong. Did you try to use it directly?")
endif()

# https://gitlab.kitware.com/cmake/cmake/-/issues/19273

# Note/question: when instrumenting a program with dynamically linked components,
#  does each component need to have a different profile-data path? I'm guessing yes?
# I don't really care about PGO for anything but the core library

# cspell:words PGOGEN PGOUSE
list(PREPEND CMAKE_CXX_FLAGS_PGOGEN ${CMAKE_CXX_FLAGS_RELEASE})
list(PREPEND CMAKE_CXX_FLAGS_PGOUSE ${CMAKE_CXX_FLAGS_RELEASE})


# trainee a target shared library
# trainer a target executable- ideally lightweight
# TODO consider making this work for training executables as well?
function(okiidoku_enable_profile_guided_optimization
	trainee # name of target to train for PGO
	trainer # name of a executable target to use for training PGO
)
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
	set(data_dir "$<TARGET_FILE_DIR:${trainee}/profile>") # TODO consider whether or not useful to make ${trainer} part of path.

	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR EMSCRIPTEN)
		# cspell:words profdata profraw
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
		if(NOT DEFINED CMAKE_CXX_COMPILER_VERSION)
			message(WARNING "could not get llvm version from `CMAKE_CXX_COMPILER_VERSION`")
			return()
		endif()
		string(REPLACE llvm_version "." ";" llvm_version_list "${CMAKE_CXX_COMPILER_VERSION}")
		list(GET llvm_version_list 0 llvm_major_version)
		find_program(LLVM_PROFDATA "llvm-profdata-${llvm_major_version}")
		add_custom_command(TARGET ${trainee} PRE_BUILD VERBATIM COMMAND_EXPAND_LISTS
			DEPENDS ""
			COMMAND "${LLVM_PROFDATA};merge;-output=\"${profdata_file}\";${data_dir}/*.profraw"
			BYPRODUCTS "${profdata_file}"
		)
		unset(LLVM_PROFDATA)
		unset(llvm_version_major)
		unset(llvm_version_list)
		unset(profdata_file)

	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		# https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
		# https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
		target_compile_options(${trainee} PRIVATE
			"$<${if_pgo}:-fprofile-dir=${data_dir}>"
			"$<${if_pgo}:-fprofile-prefix-path=${data_dir}>"
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


	# TODO find a way to only do this if the config is PgoUse?
	set(training_generator "${CMAKE_GENERATOR}")
	if(training_generator STREQUAL "Ninja Multi-Config")
		set(training_generator "Ninja")
	endif()
	# cspell:words "-DCMAKE" "-DOKIIDOKU"
	set(training_proj "${trainee}_pgo_training_ground")
	ExternalProject_Add("$${training_proj}"
		# directory options:
		SOURCE_DIR "${PROJECT_SOURCE_DIR}"
		# configure step options:
		CMAKE_GENERATOR "Ninja"
		CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
		CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}"
		CMAKE_GENERATOR_INSTANCE "${CMAKE_GENERATOR_INSTANCE}"
		CMAKE_ARGS ""
		CMAKE_CACHE_ARGS
			"-DCMAKE_CONFIGURATION_TYPES:STRING=PgoGen"
			"-DCMAKE_BUILD_TYPE:STRING=PgoGen"
			"-DOKIIDOKU_BUILD_TESTING:STRING=NO"
			"-DOKIIDOKU_BUILD_SHARED:STRING=${OKIIDOKU_BUILD_SHARED_LIBS}"
		CONFIGURE_HANDLED_BY_BUILD YES
		# build step options:
		BUILD_ALWAYS YES
		# install step options:
		INSTALL_COMMAND "" # no-op
		# test step options:
		#  none
		# output logging options:
		LOG_CONFIGURE YES LOG_BUILD YES LOG_INSTALL YES LOG_TEST YES
		LOG_OUTPUT_ON_FAILURE YES
		#
		STEP_TARGETS build install
	)
	add_dependencies(${trainee} "${training_proj}-install")

	# how do I want to run the training program?
	# - configure it to be a test and use the test step functions of ExternalProject
	# - use find_package to import the external project and run it that way?
	# - use STEP_TARGETS or ExternalProject_Add_StepTargets() to create a custom target and then add_dependencies()

	# TODO add custom command to run training program after building it.
	#  remember to mark its BYPRODUCTS and somehow make them a dependency of the optimized build
	# add_custom_target(okiidoku_pgo_training VERBATIM
	# 	COMMAND okiidoku_${trainer} # TODO
	# 	BYPRODUCTS
	# )

endfunction()
if("PgoUse" IN_LIST CMAKE_CONFIGURATION_TYPES)
	okiidoku_enable_profile_guided_optimization(okiidoku my_training_program)
endif()