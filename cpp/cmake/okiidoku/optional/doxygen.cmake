# SPDX-FileCopyrightText: 2020 David Fong
# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(DIRECTORY)
if((NOT okiidoku_IS_TOP_LEVEL) OR (NOT OKIIDOKU_BUILD_DOCS))
	return()
endif()

# include(FindDoxygen)
find_package(Doxygen 1.9) # (minimum version)
if(Doxygen_FOUND)
	list(APPEND DOXYGEN_EXCLUDE "${okiidoku_SOURCE_DIR}/external")
	list(APPEND DOXYGEN_EXCLUDE_PATTERNS
		"*/node_modules" "*/node/build"
		"*/out/build" "*/out/install"
	)
	set(DOXYGEN_OUTPUT_DIRECTORY "${okiidoku_BINARY_DIR}/docs")
	# set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${okiidoku_SOURCE_DIR}/../README.md")
	set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
	set(DOXYGEN_GENERATE_MAN YES)
	set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
	set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
	set(DOXYGEN_QUIET YES)
	list(APPEND DOXYGEN_HTML_EXTRA_STYLESHEET
		"${okiidoku_SOURCE_DIR}/cmake/okiidoku/optional/okiidoku-doxygen.css"
	)
	set(DOXYGEN_TAB_SIZE "3")
	# set(DOXYGEN_BUILTIN_STL_SUPPORT YES) # TODO try this

	block() # generate user-facing docs
		list(APPEND DOXYGEN_FILE_PATTERNS "*.hpp")
		list(APPEND DOXYGEN_EXCLUDE_PATTERNS "*/src/*")
		# list(APPEND DOXYGEN_EXTRACT_ALL YES)
		doxygen_add_docs(okiidoku_docs
				"${okiidoku_SOURCE_DIR}"
				# "${okiidoku_SOURCE_DIR}/.."
				# "${okiidoku_SOURCE_DIR}/../writings"
				# "${DOXYGEN_USE_MDFILE_AS_MAINPAGE}"
			ALL
			COMMENT "doxygen: generating user docs to file://${DOXYGEN_OUTPUT_DIRECTORY}/html/index.html"
		)
	endblock()
	install(DIRECTORY "${DOXYGEN_OUTPUT_DIRECTORY}/man"  TYPE DATA COMPONENT okiidoku_development)
	install(DIRECTORY "${DOXYGEN_OUTPUT_DIRECTORY}/html" TYPE DOC  COMPONENT okiidoku_development)
endif()