# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)

# TODO wrap this whole thing as a function taking a TARGET_NAME

if(MSVC)
	# https://docs.microsoft.com/en-us/cpp/build/profile-guided-optimizations
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR EMSCRIPTEN)
	# https://clang.llvm.org/docs/UsersManual.html#profile-guided-optimization

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	# https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
	# https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
	set(OKIIDOKU_PROFILE_DIR "$<TARGET_FILE_DIR:${TARGET_NAME}/profile>") # TODO hm wait. if output path depends on build type, how will Release pull from RelWithDebugInfo?
	if("$<CONFIG:RelWithDebugInfo>") # TODO is this the right way to do it?
		target_compile_options(${TARGET_NAME} PRIVATE
			-fprofile-dir=${OKIIDOKU_PROFILE_DIR}
			-fprofile-generate=${OKIIDOKU_PROFILE_DIR}
		)
		target_link_options(${TARGET_NAME} PRIVATE
			-fprofile-generate=${OKIIDOKU_PROFILE_DIR}
		)
	elseif("$<CONFIG:Release>")
		target_compile_options(${TARGET_NAME} PRIVATE
			-fprofile-dir=${OKIIDOKU_PROFILE_DIR}
			-fprofile-use=${OKIIDOKU_PROFILE_DIR}
			# -fprofile-correction
			# -fprofile-partial-training
		)
	endif()

	# https://gcc.gnu.org/wiki/AutoFDO/Tutorial
endif()