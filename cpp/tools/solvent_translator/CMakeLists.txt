add_executable(translator
   main.cpp
)

target_include_directories(translator PRIVATE .. ../../src)
target_include_directories(translator PUBLIC ../../include)
target_link_libraries(translator PRIVATE solvent_util solvent)