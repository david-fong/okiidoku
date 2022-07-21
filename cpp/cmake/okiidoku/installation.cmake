include_guard(DIRECTORY)

install(TARGETS
	okiidoku okiidoku_cli
	EXPORT okiidoku_installation_targets
	# RUNTIME_DEPENDENCIES # currently there are none.
	RUNTIME  COMPONENT okiidoku_runtime
	LIBRARY  COMPONENT okiidoku_runtime
	NAMELINK_COMPONENT okiidoku_development
	ARCHIVE  COMPONENT okiidoku_development
	INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

if(BUILD_SHARED_LIBS)
	set(OKIIDOKU_LIB_TYPE_NAME shared)
else()
	set(OKIIDOKU_LIB_TYPE_NAME static)
endif()

# regular installation import support:
install(EXPORT okiidoku_installation_targets
	NAMESPACE okiidoku::
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/okiidoku"
	FILE "okiidoku-${OKIIDOKU_LIB_TYPE_NAME}-targets.cmake"
	COMPONENT okiidoku_development
)
# also support importing from this repo's build-tree (no installation):
# https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#exporting-targets-from-the-build-tree
export(EXPORT okiidoku_installation_targets
	NAMESPACE okiidoku::
	FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/okiidoku-targets.cmake"
)