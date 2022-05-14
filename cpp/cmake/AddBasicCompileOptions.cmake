# settings to enable standard compliance and enable many warnings
# this should help ensure that this library supports most compilers
# and isn't less strict on itself than a very strict user's compiler
# options might be on it.

if(MSVC)
	add_compile_options(
		/nologo
		/options:strict # unrecognized compiler options are errors
		/utf-8        # /source-charset:utf-8 (for preprocessor), and /execution-charset:utf8 (for compiler)
		/permissive-  # https://discourse.cmake.org/t/cxx-extensions-and-permissive/1994
		/EHsc         # https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model#standard-c-exception-handling
		/volatile:iso # https://docs.microsoft.com/en-us/cpp/build/reference/volatile-volatile-keyword-interpretation#remarks
		/W4 #/WX      # highest warning level #and treat warnings as errors.
		/wd5030       # warning disable: "unrecognized attribute". (unfortunately needed or else MSVC warns for gnu-prefixed attributes)
		/diagnostics:caret
	)
else()
	add_compile_options(
		-Wall -Wextra -Wpedantic -Wfatal-errors #-Werror
		-Wold-style-cast -Wcast-qual -Wcast-align
		-Wformat=2
		-Wconversion # -Wsign-conversion
		-Wshadow -Woverloaded-virtual
	)
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		add_compile_options(
			-Wimplicit-fallthrough
		)
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		add_compile_options(
			-Wuseless-cast
			-Wimplicit-fallthrough=5
		)
	endif()
endif()