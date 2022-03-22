add_executable(solvent_cli
   config.cpp
   repl.cpp
   main.cpp
)

target_include_directories(solvent_cli PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(solvent_cli PRIVATE solvent_util solvent)