add_library(okiidoku_util STATIC
   console_setup.cpp
)
target_include_directories(okiidoku_util PRIVATE "${PROJECT_SOURCE_DIR}/src")