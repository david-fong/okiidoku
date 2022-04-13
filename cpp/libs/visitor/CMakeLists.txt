add_library(okiidoku_visitor STATIC
	src/okiidoku/visitor/mod.cpp
)
target_sources(okiidoku_visitor PUBLIC FILE_SET HEADERS BASE_DIRS include)
target_include_directories(okiidoku_visitor
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

find_package(Threads REQUIRED)
target_link_libraries(okiidoku_visitor
	PUBLIC okiidoku_mono # TODO.mid should this be public?
	PRIVATE Threads::Threads
)


if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
	add_subdirectory(tests)
endif()