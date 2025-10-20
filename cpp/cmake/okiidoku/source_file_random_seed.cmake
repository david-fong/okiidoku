# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)

function(get_all_targets result_var_name)
	set(targets)
	get_all_targets_recursive(result "${CMAKE_CURRENT_SOURCE_DIR}")
	# message("${result_var_name}: ${${result_var_name}}")
	set("${result_var_name}" "${result}" PARENT_SCOPE)
endfunction()

macro(get_all_targets_recursive result_var_name dir)
	get_property(dir_targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
	# message("dir_targets: ${dir_targets}")
	list(APPEND "${result_var_name}" ${dir_targets})
	get_property(subdirs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
	foreach(subdir ${subdirs})
		# message("subdir: ${subdir}")
		get_all_targets_recursive("${result_var_name}" "${subdir}")
	endforeach()
endmacro()

block()
get_all_targets(targets)
foreach(target ${targets})
	# message("target: ${target}")
	get_target_property(sources "${target}" SOURCES)
	if(sources STREQUAL "sources-NOTFOUND")
		continue()
	endif()
	set(relevant_source_langs "CXX;C")
	foreach(source ${sources})
		block()

		# skip files that aren't C/C++ compilation units:
		get_property(source_lang SOURCE "${source}" TARGET_DIRECTORY "${target}" PROPERTY LANGUAGE)
		if(NOT source_lang IN_LIST relevant_source_langs)
			continue()
		endif()
		get_property(is_header SOURCE "${source}" TARGET_DIRECTORY "${target}" PROPERTY HEADER_FILE_ONLY)
		if(is_header)
			continue()
		endif()

		get_target_property(target_src_dir "${target}" SOURCE_DIR)
		get_target_property(target_bin_dir "${target}" BINARY_DIR)

		# source -> abspath
		if(IS_ABSOLUTE "${source}")
			set(abspath "${source}")
		elseif(EXISTS "${target_src_dir}/${source}")
			set(abspath "${target_src_dir}/${source}")
		endif()

		# abspath -> relpath -> unique_id
		file(RELATIVE_PATH src_relpath "${target_src_dir}" "${abspath}")
		file(RELATIVE_PATH bin_relpath "${target_bin_dir}" "${abspath}")
		string(LENGTH "${src_relpath}" src_relpath_len)
		string(LENGTH "${bin_relpath}" bin_relpath_len)
		if("${src_relpath_len}" LESS "${bin_relpath_len}")
			set(relpath "${src_relpath}")
		else()
			set(relpath "${bin_relpath}")
		endif()
		set(unique_id "${target}:${relpath}")
		# message("${unique_id}")

		# unique_id -> compile_opts
		if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
			# https://gcc.gnu.org/onlinedocs/gcc/Developer-Options.html#index-frandom-seed
			# this also matters for matching profile data from functions with internal linkage
			# between a profile-collecting and profile-using build
			set(compile_opts "-frandom-seed=${unique_id}")
		elseif((CMAKE_CXX_COMPILER_ID MATCHES [[Clang]]) OR EMSCRIPTEN)
			set(compile_opts "-funique-source-file-identifier=${unique_id}" "-funique-source-file-names")
		endif()

		# source_file_compile_options(${source} ${compile_opts})
		set_property(SOURCE "${source}" TARGET_DIRECTORY "${target}"
			APPEND PROPERTY COMPILE_OPTIONS "${compile_opts}"
		)
		endblock()
	endforeach()
endforeach()
endblock()