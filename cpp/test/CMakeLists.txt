# todo: learn how to do functions/other mechanisms to share config between test targets
add_executable(solvent_test__equiv equiv.cpp)
# target_include_directories(equiv PRIVATE .)
target_include_directories(solvent_test__equiv PUBLIC ../src)
target_include_directories(solvent_test__equiv PUBLIC ../include)
target_link_libraries(solvent_test__equiv PRIVATE solvent_util solvent_lib)