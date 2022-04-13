add_executable(translator
   main.cpp
)

target_include_directories(translator PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(translator
   PRIVATE okiidoku_cli_utils
   PRIVATE okiidoku_visitor
)