add_executable(okiidoku_cli
   src/okiidoku_cli/config.cpp
   src/okiidoku_cli/repl.cpp
   src/okiidoku_cli/cli.main.cpp
)

target_include_directories(okiidoku_cli PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(okiidoku_cli
   PRIVATE okiidoku_cli_utils
   PRIVATE okiidoku_fmt
   PRIVATE okiidoku_visitor
)