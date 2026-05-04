#[[
set(CMAKE_PROJECT_<project_name>_INCLUDE "${okiidoku_SOURCE_DIR}/cmake/okiidoku/dep_project_include.cmake")
do this to make a dependency be Release for RelWithDebInfo builds, and MinSizeRel for Debug builds.
\internal: this potentially could be done for Ninja builds with cross-config.
]]
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_MINSIZEREL}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE}")