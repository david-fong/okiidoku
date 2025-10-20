https://github.com/actions/runner-images/issues/9491
https://stackoverflow.com/q/77894856/11107541
https://github.com/google/sanitizers/wiki/SanitizerCommonFlags

why is it so complicated to run with sanitizers?

some experimentation with `GET_RUNTIME_DEPENDENCIES`. doesn't help. can't resolve clang sharedlib asan. can't clang make things a bit easier and add it to it's output RPATH?

```cmake
file(WRITE "${CMAKE_BINARY_DIR}/test.cmake" [[
file(GET_RUNTIME_DEPENDENCIES
	RESOLVED_DEPENDENCIES_VAR resolved
	UNRESOLVED_DEPENDENCIES_VAR unresolved
	LIBRARIES ${target_file}
)
message("resolved: ${resolved}")
message("unresolved: ${unresolved}")
file(WRITE "${out_path}" "resolved: ${resolved}\nunresolved: ${unresolved}")
]])
add_custom_command(
	OUTPUT "${CMAKE_BINARY_DIR}/test.txt"
	COMMAND "${CMAKE_COMMAND}" "-D target_file=$<TARGET_FILE:okiidoku>" "-D out_path=\"${CMAKE_BINARY_DIR}/test.txt\"" -P "${CMAKE_BINARY_DIR}/test.cmake"
	DEPENDS okiidoku
	VERBATIM COMMAND_EXPAND_LISTS USES_TERMINAL
)
add_custom_target(testing ALL DEPENDS "${CMAKE_BINARY_DIR}/test.txt")
```