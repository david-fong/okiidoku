include_guard(DIRECTORY)

if(MSVC)
	add_compile_options(/EHsc)
	# https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model#standard-c-exception-handling
	# must be consistent for whole program(s). do not hijack user's choice.
	# This project doesn't throw exceptions, so any model should be okay.
	# TODO wat https://gitlab.kitware.com/cmake/cmake/-/issues/20610
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	add_compile_options(
		-fno-semantic-interposition # disable ELF external overrides for more optimization (on by default for LLVM)
	)
endif()

if(NOT MSVC)
	add_compile_options(
		-march=native # enable optimizing for the compiling machine's instruction set architecture.
	)
endif()