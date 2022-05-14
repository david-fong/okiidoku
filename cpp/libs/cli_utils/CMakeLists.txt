add_library(okiidoku_cli_utils STATIC
   src/okiidoku_cli_utils/console_setup.cpp
)
target_include_directories(okiidoku_cli_utils PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")