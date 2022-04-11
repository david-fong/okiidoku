add_library(okiidoku_visitor STATIC
	src/okiidoku/visitor/mod.cpp
)

target_include_directories(okiidoku_visitor
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
	PUBLIC
	# $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_sources(okiidoku_visitor PUBLIC FILE_SET HEADERS BASE_DIRS include)


find_package(Threads REQUIRED)
target_link_libraries(okiidoku_visitor
	PUBLIC Threads::Threads
)


if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
	add_subdirectory(tests)
endif()