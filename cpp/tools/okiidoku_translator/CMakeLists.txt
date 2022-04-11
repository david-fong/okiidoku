add_executable(translator
   main.cpp
)

target_include_directories(translator PRIVATE "${PROJECT_SOURCE_DIR}/test" "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(translator PRIVATE okiidoku_cli_utils okiidoku)