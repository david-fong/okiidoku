# todo: learn how to do functions/other mechanisms to share config between test targets
add_executable(solvent_test tests.cpp)
target_compile_options(solvent_test
	PUBLIC
	# -fsanitize=address # ? https://youtu.be/4pKtPWcl1Go?t=12m49s
)
# target_include_directories(morph PRIVATE .)
target_include_directories(solvent_test PUBLIC ../src)
target_include_directories(solvent_test PUBLIC ../include)
target_link_libraries(solvent_test PRIVATE solvent_util solvent)