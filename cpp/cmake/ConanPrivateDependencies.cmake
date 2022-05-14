# following instructions at https://docs.conan.io/en/latest/howtos/cmake_launch.html
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
	message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
	file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
	"${CMAKE_BINARY_DIR}/conan.cmake")
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(
	REQUIRES ${OKIIDOKU_CONAN_DEPENDENCIES}
	BASIC_SETUP CMAKE_TARGETS KEEP_RPATHS NO_OUTPUT_DIRS
	BUILD missing # build deps with no pre-built binary artifacts
)