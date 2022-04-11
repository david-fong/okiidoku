add_library(okiidoku_cli_utils STATIC
   src/okiidoku_cli_utils/console_setup.cpp
)
target_sources(okiidoku_cli_utils PUBLIC FILE_SET HEADERS BASE_DIRS include)