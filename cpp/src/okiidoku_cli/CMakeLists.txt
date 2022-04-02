add_executable(okiidoku_cli
   config.cpp
   repl.cpp
   main.cpp
)

target_include_directories(okiidoku_cli PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(okiidoku_cli PRIVATE okiidoku_util okiidoku)