# https://alexreinking.com/blog/building-a-dual-shared-and-static-library-with-cmake.html
# https://www.reddit.com/r/cpp/comments/m0rt7v/building_a_dual_shared_and_static_library_with/
# TODO.low try Alex's tutorial things for installation utilities.
add_library(okiidoku #[[ uses BUILD_SHARED_LIBS ]] "")
add_library(okiidoku::okiidoku ALIAS okiidoku)
target_compile_features(okiidoku PUBLIC cxx_std_20)
set_target_properties(okiidoku PROPERTIES
	# https://www.youtube.com/watch?v=m0DwB4OvDXk&ab_channel=CppCon&t=17m13s
	VERSION   ${okiidoku_VERSION}
	SOVERSION ${okiidoku_VERSION_MAJOR}
	CXX_STANDARD_REQUIRED TRUE
	CXX_EXTENSIONS OFF
	INTERPROCEDURAL_OPTIMIZATION TRUE
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN TRUE
)
add_subdirectory(src/okiidoku)

include(GenerateExportHeader)
generate_export_header(okiidoku EXPORT_FILE_NAME include/okiidoku/detail/export.h)
target_compile_definitions(okiidoku PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:OKIIDOKU_STATIC_DEFINE>")

target_include_directories(okiidoku
	PUBLIC  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
	PUBLIC  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
	PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
)

# Future Note: if switching back to FILE_SET, make sure headers get installed. not sure why they weren't when previously using FILE_SET. maybe each header file needs to be listed out explicitly?
install(DIRECTORY
	"${CMAKE_CURRENT_BINARY_DIR}/include/okiidoku"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/okiidoku"
	TYPE INCLUDE
	COMPONENT okiidoku_development
)

target_link_libraries(okiidoku
	PUBLIC  okiidoku_compile_options_public
	PRIVATE okiidoku_compile_options_private
	PRIVATE range-v3::range-v3
)

if(okiidoku_IS_TOP_LEVEL)
	add_subdirectory(examples EXCLUDE_FROM_ALL)
	add_subdirectory(tests)
endif()