add_library(solvent_util STATIC
   console_setup.cpp
)
target_include_directories(solvent_util PRIVATE "${PROJECT_SOURCE_DIR}/src")