add_executable(okiidoku_translator
   main.cpp
)

target_include_directories(okiidoku_translator PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(okiidoku_translator
   PRIVATE okiidoku_cli_utils
   PRIVATE okiidoku
)