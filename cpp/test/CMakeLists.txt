# todo: learn how to do functions/other mechanisms to share config between test targets
add_executable(solvent_test__morph morph.cpp)
# target_include_directories(morph PRIVATE .)
target_include_directories(solvent_test__morph PUBLIC ../src)
target_include_directories(solvent_test__morph PUBLIC ../include)
target_link_libraries(solvent_test__morph PRIVATE solvent_util solvent_lib)