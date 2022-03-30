add_library(ookiidoku_util STATIC
   console_setup.cpp
)
target_include_directories(ookiidoku_util PRIVATE "${PROJECT_SOURCE_DIR}/src")