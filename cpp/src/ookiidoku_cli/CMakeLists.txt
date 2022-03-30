add_executable(ookiidoku_cli
   config.cpp
   repl.cpp
   main.cpp
)

target_include_directories(ookiidoku_cli PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(ookiidoku_cli PRIVATE ookiidoku_util ookiidoku)